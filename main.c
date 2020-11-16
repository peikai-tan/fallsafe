#include "main.h"

#include "common/queue.h"
#include "common/vector3.h"

#include "utils/time.h"
#include "utils/mqtt-sender.h"

#include "genann/combined_classifier.h"

#define DEBUG

/* Psuedo code:
 *
 * Check timer thingy
 * If timer triggers, read data
    * If queue is full, dequeue
    * Then, queue data 
 * If queue is pull, push data into model, get result
 * If result is fall, wait for user input, blink red 
 * Else, continue running
 */

// Configurations
// TODO: allow CLI args to overide these
#define GatheringFrequency 30
static bool continueProgram = true;
static double dataGatherIntervalMS = 1000.0 / GatheringFrequency;
static unsigned int updateIntervalMicroSeconds = 100;
static size_t queueTarget = 60;

typedef enum fallsafe_state
{
    INITIAL,
    NORMAL,
    FALLEN
} FallsafeState;

typedef struct fallsafe_context
{
    double applicationStartTime;
    double deltaTime;
    double unixTime;
    double actualInterval;
    Queue acceleroDataset;
    Vector3 *acceleroDataChunk;
    double *unrolledDataChunk;
    int joystickFB;
    struct pollfd evpoll;
    int sensehatfbfd;
    uint16_t *sensehatLEDMap;
    FallsafeState state;
    Classifier classifier;
    ActivityState activityState;
} FallsafeContext;

/**
 * Read data from accelerometer and push to dataset queue 
*/
static Vector3 gather_data(const FallsafeContext *context)
{
    Vector3 acceleroData;
    shGet2GAccel(&acceleroData);
    queue_enqueue(context->acceleroDataset, &acceleroData);

#if defined(DEBUG)
    printf("[Gather Data] unixtime: %0.0lf actualinterval: %0.3lf queue: %zu accelerometer: ", context->unixTime, context->actualInterval, context->acceleroDataset->length);
    vector3_print(&acceleroData);
#endif // DEBUG

    return acceleroData;
}

static ActivityState process_data(const FallsafeContext *context)
{
    ActivityState output;
    // ML processing
    queue_peekRange(context->acceleroDataset, queueTarget, (void **)&context->acceleroDataChunk);
    // Unrolling data
    for (size_t i = 0; i < queueTarget; i++)
    {
        context->unrolledDataChunk[i] = context->acceleroDataChunk[i].x;
    }
    for (size_t i = 0; i < queueTarget; i++)
    {
        context->unrolledDataChunk[i + queueTarget] = context->acceleroDataChunk[i].y;
    }
    for (size_t i = 0; i < queueTarget; i++)
    {
        context->unrolledDataChunk[i + queueTarget * 2] = context->acceleroDataChunk[i].z;
    }

    // #if defined(DEBUG)

    //     if (context->unixTime - context->applicationStartTime > 5000)
    //     {
    //         for (size_t i = 0; i < queueTarget; i++)
    //         {
    //             printf("%d ", (int)i);
    //             vector3_print(&context->acceleroDataChunk[i]);
    //         }

    //         for (size_t i = 0; i < queueTarget * 3; i++)
    //         {
    //             printf("%d %lf\n", (int)i, context->unrolledDataChunk[i]);
    //         }
    //         getchar();
    //     }

    // #endif // DEBUG

    int state = classifier_predict(context->classifier, context->unrolledDataChunk);

    switch (state)
    {
    // Fall
    case 0:
        return FALLING;
    // Walk
    case 1:
        return WALKING;
    // Run
    case 2:
        return RUNNING;
    // Station
    case 3:
        return STATIONARY;
    // Jump
    case 4:
        return JUMPING;
    default:
        fprintf(stderr, "[ERROR] Received other values\n");
        return STATIONARY;
    }
}

static void send_thingsboardAccel(FallsafeContext *context, Vector3 data)
{
    // Send data to thingsboard
    mqtt_send_vector3(&data, (long long)context->unixTime);
}

static void send_thingsboardState(FallsafeContext *context, ActivityState state, double time_ms)
{
    // Send data to thingsboard
    mqtt_send_activity(state, (long long)time_ms);
}

static void check_process_data(FallsafeContext *context)
{
    static double interval = 1000;
    static double timepassed = 0;
    static ActivityState previousState;

    timepassed += context->actualInterval;

    if (timepassed < interval)
    {
        return;
    }

    timepassed = 0;

    // Check and perform ML data processing

    ActivityState state = process_data(context);
    context->activityState = state;
    send_thingsboardState(context, state, context->unixTime);
    printf("Proccessed State: %d ================================================\n", state);
    // Show LED
    if (previousState != state)
    {
        setMap(0x0000, context->sensehatLEDMap, &context->sensehatfbfd);
        previousState = state;

        if (state == FALLING)
        {
            context->state = FALLEN;
            printf("[Waiting Input] unixtime: %0.0lf\n", context->unixTime);
        }
    }
}

static void perform_task(FallsafeContext *context)
{
    // Gather the sensor data at current moment instant
    Vector3 acceleroData = gather_data(context);

    send_thingsboardAccel(context, acceleroData);

    // Check if queue is at the target length for processing
    if (context->acceleroDataset->length <= queueTarget * 2)
    {
        context->state = INITIAL;
        return;
    }
    context->state = NORMAL;

    // Dequeue buffer
    Vector3 data;
    queue_dequeue(context->acceleroDataset, &data);

    check_process_data(context);
    drawActivity(context->activityState, context->sensehatLEDMap, &context->sensehatfbfd);
}

/**
 * To runs every update to check if it is time to gather data and process them
*/
static void check_perform_task(FallsafeContext *context)
{
    static double timepassed = 0;
    static double previousTime = 0;
    timepassed += context->deltaTime;
    if (timepassed >= dataGatherIntervalMS)
    {
        previousTime = previousTime == 0 ? context->applicationStartTime : previousTime;
        context->actualInterval = context->unixTime - previousTime;
        previousTime = context->unixTime;
        perform_task(context);
        timepassed -= dataGatherIntervalMS;
        if (timepassed > dataGatherIntervalMS)
        {
            timepassed = 0;
        }
    }
}

/**
 * If falling is detected, program state will changed to waiting user input to indicate whether it is an false positive.
 * If no input is detected after a certain period of time, an alert to external output will be triggered
*/
static void await_userinput(FallsafeContext *context)
{
    static Joystick joystick;
    static int currentSelection = -1;

    ActivityState selections[3] = {STATIONARY, WALKING, RUNNING};

    while (poll(&context->evpoll, 1, 0) > 0)
    {
        if (readJoystick(&context->joystickFB, &joystick))
        {
            if (joystick.state == PRESSED)
            {
                printf("[INFO] Joystick PRESSED direction: %s=====================================================\n", joystick.direction);
            }
            if (joystick.state == HOLD)
            {
                printf("[INFO] Joystick HOLD direction: %s=====================================================\n", joystick.direction);
            }
            if (joystick.state == RELEASE)
            {
                printf("[INFO] Joystick RELEASE direction: %s=====================================================\n", joystick.direction);
                switch (joystick.dir)
                {
                case ENTER:
                    context->state = INITIAL;
                    context->activityState = STATIONARY;
                    queue_destroy(context->acceleroDataset);
                    context->acceleroDataset = queue_new(Vector3, queueTarget);
                    setMap(0x0000, context->sensehatLEDMap, &context->sensehatfbfd);

                    // Reenforced learning

                    break;


                case UP:
                case RIGHT:
                    currentSelection = (currentSelection + 1) % 3;
                    setMap(0x0000, context->sensehatLEDMap, &context->sensehatfbfd);
                    drawActivity(selections[currentSelection], context->sensehatLEDMap, &context->sensehatfbfd);
                    break;

                case DOWN:
                case LEFT:
                    currentSelection = currentSelection - 1 < 0 ? 2 : (currentSelection - 1);
                    setMap(0x0000, context->sensehatLEDMap, &context->sensehatfbfd);
                    drawActivity(selections[currentSelection], context->sensehatLEDMap, &context->sensehatfbfd);
                    break;
                }
            }
        }
        else
        {
            fprintf(stderr, "[ERROR] Joystick read error. \n");
        }
    }
}

static void update_rolling_led(FallsafeContext *context)
{
    static const double interval = 1000.0 / 15;
    static int previousPosition = 0;
    static int currentPosition = 0;
    static double timepassed = 0;
    timepassed += context->deltaTime;
    if (timepassed > interval)
    {
        shSetPixel(previousPosition, 0, 0, 1, context->sensehatLEDMap, &context->sensehatfbfd);
        shSetPixel(currentPosition, 0, 0x4000, 1, context->sensehatLEDMap, &context->sensehatfbfd);
        shSetPixel(7, previousPosition, 0, 1, context->sensehatLEDMap, &context->sensehatfbfd);
        shSetPixel(7, currentPosition, 0x4000, 1, context->sensehatLEDMap, &context->sensehatfbfd);
        shSetPixel(0, 7 - previousPosition, 0, 1, context->sensehatLEDMap, &context->sensehatfbfd);
        shSetPixel(0, 7 - currentPosition, 0x4000, 1, context->sensehatLEDMap, &context->sensehatfbfd);
        shSetPixel(7 - previousPosition, 7, 0, 1, context->sensehatLEDMap, &context->sensehatfbfd);
        shSetPixel(7 - currentPosition, 7, 0x4000, 1, context->sensehatLEDMap, &context->sensehatfbfd);
        previousPosition = currentPosition;
        currentPosition = (currentPosition + 1) % 7;
        timepassed -= interval;
        if (timepassed > interval)
        {
            timepassed = 0;
        }
    }
}

/**
 * Main update loop
*/
static void update(FallsafeContext *context)
{
    switch (context->state)
    {
    case INITIAL:
    case NORMAL:
        update_rolling_led(context);
        check_perform_task(context);
        break;
    case FALLEN:
        await_userinput(context);
        break;
    default:
        fprintf(stderr, "[ERROR] Invalid State: %d\n", context->state);
        break;
    }
}

/**
 * Handles CTRL+C to gracefully exit program
*/
void exit_handler(int signum)
{
    printf("\n[INFO] Handling SIGINT: %d\n", signum);
    continueProgram = false;
}

int main(int agc, char **argv)
{
    Classifier classifier;

    FallsafeContext context;
    double previousTime;
    double currentTime;
    int toleranceTime = 0;

    context.acceleroDataset = queue_new(Vector3, queueTarget * 3.25);
    context.acceleroDataChunk = (Vector3 *)malloc(sizeof(Vector3) * queueTarget * 3);
    context.unrolledDataChunk = (double *)malloc(sizeof(double) * queueTarget * 3);
    context.state = INITIAL;
    context.evpoll.events = POLLIN;

    // Set up programming termination handler
    signal(SIGINT, exit_handler);
    // Set up wiring pi
    wiringPiSetupSys();

    // Set up MQTT Wrapper
    mqtt_open_socket();
    mqtt_setup_client();

    // Set up sensehat sensors
    if (shInit(1, &context.sensehatfbfd) == 0)
    {
        fprintf(stderr, "[ERROR] Unable to open sense, is it connected?\n");
        return -1;
    }
    if (mapLEDFrameBuffer(&context.sensehatLEDMap, &context.sensehatfbfd) == 0)
    {
        fprintf(stderr, "[ERROR] Unable to map LED to Frame Buffer. \n");
        return -1;
    }
    if (initJoystick(&context.joystickFB) == -1)
    {
        fprintf(stderr, "[ERROR] Unable to open joystick event\n");
        return -1;
    }

    context.classifier = classifier_new();
    if (context.classifier == NULL)
    {
        fprintf(stderr, "[ERROR] Unable to initialize model\n");
    }

    context.evpoll.fd = context.joystickFB;
    context.applicationStartTime = get_unixtime_ms();
    previousTime = get_monotonicclock_ms();

    while (continueProgram)
    {
        delayMicroseconds(updateIntervalMicroSeconds + toleranceTime);
        currentTime = get_monotonicclock_ms();
        context.deltaTime = currentTime - previousTime;
        previousTime = currentTime;
        context.unixTime = get_unixtime_ms();

        update(&context);
    }

    free(context.acceleroDataChunk);
    free(context.unrolledDataChunk);
    queue_destroy(context.acceleroDataset);
    classifier_destroy(context.classifier);
    setMap(0x0000, context.sensehatLEDMap, &context.sensehatfbfd);
    shShutdown(&context.sensehatfbfd, context.sensehatLEDMap);
    mqtt_dispose();
    printf("[INFO] Program terminated\n");
    return 0;
}

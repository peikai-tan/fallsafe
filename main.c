#include "main.h"
#include "configuration.h"

#include "lib/sensehat/sensehat.h"

#include "common/queue.h"
#include "common/vector3.h"

#include "utils/time.h"
#include "utils/mqtt-sender.h"

#include "genann/combined_classifier.h"

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
// Define constant for frequency in Hz
#define GatheringFrequency 30
// Global variables for program-wide settings
static bool continueProgram = true;
static double dataGatherIntervalMS = 1000.0 / GatheringFrequency;
static unsigned int updateIntervalMicroSeconds = 100;
static size_t queueTarget = 60;

// Enum far various activity states
typedef enum fallsafe_state
{
    // Initial data gathering state
    INITIAL,
    // Activity detection state
    NORMAL,
    // Fallen state awaiting user input
    FALLEN
} FallsafeState;

// FallSafe 'class' to store and pass values locally
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
    char *mqttAccessToken;
    char *emailAddress;
    bool enableLED;
    double processingIntervalMS;
} FallsafeContext;

/**
 * gather_data() - Read data from accelerometer and push to dataset queue 
 * @context: FallsafeContext class that contains program-wide variables
 *
 * Gets data from accelerometer and store in program-wide context class
 *
 * Return: Vector3 data after it has been succesfully read from the sensor
*/
static Vector3 gather_data(const FallsafeContext *context)
{
    Vector3 acceleroData;
    acceleroData.x = 0;
    acceleroData.y = 0;
    acceleroData.z = 0;
    // Get accelerometer data from sensehat and store into vector 3
    shGet2GAccel(&acceleroData);
    // Enqueue vector 3 data into fallsafe class
    queue_enqueue(context->acceleroDataset, &acceleroData);

#if defined(DEBUG)
    printf("[Gather Data] unixtime: %0.0lf actualinterval: %0.3lf queue: %zu accelerometer: ", context->unixTime, context->actualInterval, context->acceleroDataset->length);
    vector3_print(&acceleroData);
#endif // DEBUG

    return acceleroData;
}

/**
 * process_data() -  Place data chunk into an array and pass data into model
 * @context: FallsafeContext class that contains program-wide variables
 *
 * Unroll accelerometer data into an array and pass into classifier function.
 *
 * Return: ActivityState to display corresponding alphabet on the LED matrix
 */
static ActivityState process_data(const FallsafeContext *context)
{
    // ML processing
    queue_peekRange(context->acceleroDataset, queueTarget, (void **)&context->acceleroDataChunk);
    // Unrolling data:
    // x values
    for (size_t i = 0; i < queueTarget; i++)
    {
        context->unrolledDataChunk[i] = context->acceleroDataChunk[i].x;
    }
    // y values
    for (size_t i = 0; i < queueTarget; i++)
    {
        context->unrolledDataChunk[i + queueTarget] = context->acceleroDataChunk[i].y;
    }
    // z values
    for (size_t i = 0; i < queueTarget; i++)
    {
        context->unrolledDataChunk[i + queueTarget * 2] = context->acceleroDataChunk[i].z;
    }

    // Store predicted state
    int state = classifier_predict(context->classifier, context->unrolledDataChunk);

    // Convert state from classifier into program state
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

/**
 * send_thingsboardAccel() - Send accelerometer data to thingsboard
 * @context: FallsafeContext class that contains program-wide variables
 * @data: Vector3 accelerometer value
 *
 * Calls mqtt function to send vector 3 data to thingsboard
 */
static void send_thingsboardAccel(FallsafeContext *context, Vector3 data)
{
    // Send data to thingsboard
    mqtt_send_vector3(&data, (long long)context->unixTime);
}

/** 
 * send_thingsboardState() - Send predicted state to thingsboard
 * @context: FallsafeContext class that contains program-wide variables
 * @state: ActivityState that contains the current predicted activity
 * @time_ms: Current time in Unix offset
 *
 * Send predicted activity state and time in milliseconds
 */
static void send_thingsboardState(FallsafeContext *context, ActivityState state)
{
    // Send data to thingsboard
    mqtt_send_activity(state, (long long)context->unixTime);
}

/**
 * check_process_data() - Determine actions after checking data
 * @context: FallsafeContext class that contains program-wide variables
 *
 * Trigger specific actions according to result of data check.
 * This does not trigger if it has not collected enough data on first start.
 */
static void check_process_data(FallsafeContext *context)
{
    static double timepassed = 0;
    static ActivityState previousState;

    // Count time with interval
    timepassed += context->actualInterval;

    // Controls data checking
    if (timepassed < context->processingIntervalMS)
    {
        return;
    }

    // Reset timepassed counter
    timepassed = 0;

    // Check and perform ML data processing
    ActivityState state = process_data(context);
    // Store predicted state into context
    context->activityState = state;
    send_thingsboardState(context, state);
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

/**
 * perform_task() - Trigger data gathering and sending of data to Thingsboard
 * @context: FallsafeContext class that contains program-wide variables
 *
 * Triggers data collection from accelerometer and send it to thingsboard
 * If queue has reached maximum size, dequeue 1 set of data
 */
static void perform_task(FallsafeContext *context)
{
    // Gather the sensor data at current moment 
    Vector3 acceleroData = gather_data(context);

    // Send accelerometer data to thingsboard
    send_thingsboardAccel(context, acceleroData);

    // Check if queue is at the target length for processing
    if (context->acceleroDataset->length <= queueTarget)
    {
        context->state = INITIAL;
        return;
    }
    context->state = NORMAL;

    // Dequeue buffer
    Vector3 data;
    queue_dequeue(context->acceleroDataset, &data);

    // Pass data for processing
    check_process_data(context);
    drawActivity(context->activityState, context->sensehatLEDMap, &context->sensehatfbfd);
}

/**
 * check_perform_task() - To runs every update to check if it is time to 
 * gather data and process them
 * @context: FallsafeContext class that contains program-wide variables
 *
 * Determine time interval for data gathering and processing
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
 * send_externalalert() - Sends an email to the specified target email
 * when a fall has been detected with no user input
 * @context: FallsafeContext class that contains program-wide variables
 * 
 * Function sends an email to email in context via a curl get request to Google
 * API
 */
static void send_externalalert(FallsafeContext *context)
{
    if (context->emailAddress == NULL)
    {
        printf("[Email Sending] No Email Provided\n");
        return;
    }
    char endPoint[256];
    char outputBuffer[256];
    // Places email address into post request and store into endpoint
    sprintf(endPoint, "curl -sL https://script.google.com/macros/s/AKfycbzbGoq2XFMekzfR9OmpScNKFZ91G-CENr1Np6yWnljyPKCLjJDh/exec?email=%s", context->emailAddress);
    // Opens a process to execute curl command
    FILE *curlProcess = popen(endPoint, "r");
    // Reads http response
    fscanf(curlProcess, "%255s", outputBuffer);
    // Close process
    pclose(curlProcess);
    printf("[Email Sending] Recipient: %s Status: %s\n", context->emailAddress, outputBuffer);
}

/**
 * await_userinput() - Triggers when falling has been detected 
 * @context: FallsafeContext class that contains program-wide variables
 *
 * If falling is detected, program state will changed to waiting 
 * user input to indicate whether it is an false positive.
 * If no input is detected after a certain period of time, 
 * an alert to external output will be triggered
*/
static void await_userinput(FallsafeContext *context)
{
    static ActivityState selections[] = {WALKING, RUNNING, STATIONARY, UNKNOWN};
    static Joystick joystick;
    static int currentSelection = -1;
    static bool sent = false;
    static bool interacted = false;
    static double timePassedMS = 0;

    timePassedMS += context->deltaTime;

    // Triggers email sending after specified time 
    if (timePassedMS >= 5000 && !sent && !interacted)
    {
        send_externalalert(context);
        sent = true;
        interacted = true;
    }

    // Waits for user input 
    while (poll(&context->evpoll, 1, 0) > 0)
    {
        // If successful read from joystick
        if (readJoystick(&context->joystickFB, &joystick))
        {
            interacted = true;
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
                // Selects option and resets everything
                case ENTER:
                    context->state = INITIAL;
                    context->activityState = STATIONARY;
                    queue_destroy(context->acceleroDataset);
                    context->acceleroDataset = queue_new(Vector3, queueTarget + 1);
                    setMap(0x0000, context->sensehatLEDMap, &context->sensehatfbfd);
                    timePassedMS = 0;
                    interacted = false;
                    sent = false;
                    if (currentSelection == 3)
                    {
                        printf("[INFO] Ignoring unknown activity\n");
                        break;
                    }

                    // Reinforced learning
                    classifier_reinforce(context->classifier, context->unrolledDataChunk, currentSelection + 1);
                    printf("[INFO] Reinforced with %d\n", currentSelection + 1);
                    currentSelection = -1;

                    break;

                // Does nothing
                case UP:
                // Selects next option
                case RIGHT:
                    currentSelection = (currentSelection + 1) % 4;
                    setMap(0x0000, context->sensehatLEDMap, &context->sensehatfbfd);
                    drawActivity(selections[currentSelection], context->sensehatLEDMap, &context->sensehatfbfd);
                    break;

                // Does nothing
                case DOWN:
                // Selects previous option
                case LEFT:
                    currentSelection = currentSelection - 1 < 0 ? 3 : (currentSelection - 1);
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

/**
 * update_rolling_led() - Set pixel for LED Array
 * @context: FallsafeContext class that contains program-wide variables
 * 
 * Spawns red-lit leds and animates it around the border of the LED matrix.
 */
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
 * update() - Main update loop
 * @context: FallsafeContext class that contains program-wide variables
 * 
 * Starter function that triggers main parts of the program depending on the
 * state of the program.
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
 * exit_handler() - Handles CTRL+C to gracefully exit program
 * @signum - Signal interrupt value
 *
 * Controls behaviour of CTRL+C
*/
void exit_handler(int signum)
{
    printf("\n[INFO] Handling SIGINT: %d\n", signum);
    continueProgram = false;
}

int main(int argc, char **argv)
{
#if defined(DEBUG)
    puts("DEBUG MODE");
#endif // DEBUG

    struct configuration config = parse_command_line(argc, argv);

    FallsafeContext context;
    double previousTime;
    double currentTime;
    int toleranceTime = 0;

    context.acceleroDataset = queue_new(Vector3, queueTarget + 1);
    context.acceleroDataChunk = (Vector3 *)malloc(sizeof(Vector3) * queueTarget * 3);
    context.unrolledDataChunk = (double *)malloc(sizeof(double) * queueTarget * 3);
    context.state = INITIAL;
    context.evpoll.events = POLLIN;
    context.emailAddress = config.emailAddress;
    context.mqttAccessToken = config.mqttAccessToken;
    context.processingIntervalMS = config.processingIntervalMS;
    context.activityState = STATIONARY;

    // Set up program termination handler
    signal(SIGINT, exit_handler);
    // Set up wiring pi
    wiringPiSetupSys();

    // Set up MQTT Wrapper
    mqtt_open_socket();
    mqtt_setup_client(context.mqttAccessToken);

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

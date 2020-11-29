#include "main.h"
#include "configuration.h"
#include "fallsafe.h"

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

/* Program's heartbeat switch */
static bool continueProgram = true;
/* Program's heartbeat interval */
static unsigned int updateIntervalMicroSeconds = 1000;

/* pthread condition and locks for software timer interrupt for LED Render */
pthread_cond_t pthread_timer_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t pthread_timer_lock = PTHREAD_MUTEX_INITIALIZER;

FallsafeContext context;

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
    case FALLSAFE_INITIAL:
    case FALLSAFE_NORMAL:
        fallsafe_check_perform_task(context);
        break;
    case FALLSAFE_FALLEN:
        fallsafe_await_userinput(context);
        break;
    default:
        fprintf(stderr, "[ERROR] Invalid State in update: %d\n", context->state);
        break;
    }
}

static void render(FallsafeContext *context)
{
    switch (context->state)
    {
    case FALLSAFE_INITIAL:
        fallsafe_update_rolling_led(context);
        break;
    case FALLSAFE_NORMAL:
        setMap(0x0000, context->sensehatLEDMap, &context->sensehatfbfd);
        fallsafe_update_rolling_led(context);
        drawActivity(context->activityState, context->sensehatLEDMap, &context->sensehatfbfd);
        break;
    case FALLSAFE_FALLEN:
        setMap(0x0000, context->sensehatLEDMap, &context->sensehatfbfd);
        drawActivity(context->activityState, context->sensehatLEDMap, &context->sensehatfbfd);
        break;
    default:
        fprintf(stderr, "[ERROR] Invalid State in render: %d\n", context->state);
        break;
    }
}

static void timer_interupt_handler(int signum)
{
    // Call the LED render function
    render(&context);
    // Signal the thread to check if to continue the program execution
    pthread_cond_signal(&pthread_timer_cond);
}

static void *render_thread(void *args)
{
    struct itimerval timerval;
    timerval.it_value.tv_sec = 0;
    timerval.it_value.tv_usec = (long long)(1000.0 / 15 * 1000);
    timerval.it_interval.tv_sec = 0;
    timerval.it_interval.tv_usec = (long long)(1000.0 / 15 * 1000);

    struct sigaction action = {0};
    action.sa_handler = &timer_interupt_handler;

    sigaction(SIGALRM, &action, NULL);

    if (setitimer(ITIMER_REAL, &timerval, NULL))
    {
        fprintf(stderr, "Failure starting itimer\n");
    }

    while (continueProgram)
    {
        pthread_cond_wait(&pthread_timer_cond, &pthread_timer_lock);
    }

    return NULL;
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
    puts("[INFO] DEBUG MODE");
#endif // DEBUG

    Configuration config = parse_command_line(argc, argv);

    double previousTime;
    double currentTime;

    context.acceleroDataset = queue_new(Vector3, queueTarget + 1);
    context.acceleroDataChunk = (Vector3 *)malloc(sizeof(Vector3) * queueTarget * 3);
    context.unrolledDataChunk = (double *)malloc(sizeof(double) * queueTarget * 3);
    context.state = FALLSAFE_INITIAL;
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
    context.evpoll.fd = context.joystickFB;

    context.classifier = classifier_new();
    if (context.classifier == NULL)
    {
        fprintf(stderr, "[ERROR] Unable to initialize model\n");
    }

    pthread_t renderThreadId;
    pthread_create(&renderThreadId, NULL, render_thread, &context);

    context.applicationStartTime = get_unixtime_ms();
    previousTime = get_monotonicclock_ms();

    while (continueProgram)
    {
        delayMicroseconds(updateIntervalMicroSeconds);
        currentTime = get_monotonicclock_ms();
        context.deltaTime = currentTime - previousTime;
        previousTime = currentTime;
        context.unixTime = get_unixtime_ms();

        update(&context);
    }

    pthread_join(renderThreadId, NULL);
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

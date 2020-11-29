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
static volatile bool continueProgram = true;
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
        // INITIAL state gathers data but not processing as there arent enough data in the queue
        // If queue is long enough (60, past 2 seconds at 30Hz) will start process start to detect activity state
        fallsafe_check_perform_task(context);
        break;
    case FALLSAFE_FALLEN:
        // Wait for user input for reinforcement learning after a fall is detected
        fallsafe_await_userinput(context);
        break;
    default:
        fprintf(stderr, "[ERROR] Invalid State in update: %d\n", context->state);
        break;
    }
}

/**
 * render() - to be called by software timer interrupt at 15 Hz.
 * This animates the Sense Hat LED Matrix
*/
static void render(FallsafeContext *context)
{
    switch (context->state)
    {
    case FALLSAFE_INITIAL:
        // Show rolling LED to show that the program is running smoothly
        fallsafe_update_rolling_led(context);
        break;
    case FALLSAFE_NORMAL:
        // Clear the LED to remove the previous state drawing
        setMap(0x0000, context->sensehatLEDMap, &context->sensehatfbfd);
        // Still Show rolling LED to show that the program is running smoothly
        fallsafe_update_rolling_led(context);
        // Draw the processed normal state activity letter: S for stationary, W for walking, R for Running
        drawActivity(context->activityState, context->sensehatLEDMap, &context->sensehatfbfd);
        break;
    case FALLSAFE_FALLEN:
        // Clear the previous state drawing
        setMap(0x0000, context->sensehatLEDMap, &context->sensehatfbfd);
        // Draw the fall activity or then the user input activity for reinforcement learning selections
        drawActivity(context->activityState, context->sensehatLEDMap, &context->sensehatfbfd);
        break;
    default:
        fprintf(stderr, "[ERROR] Invalid State in render: %d\n", context->state);
        break;
    }
}

/**
 * timer_interupt_handler() - The software timer interrupt routine handler to call the LED render function
*/
static void timer_interupt_handler(int signum)
{
    (void)signum; // Not used
    // Call the LED render function
    render(&context);
    // Signal the thread to check if to continue the program execution
    pthread_cond_signal(&pthread_timer_cond);
}

/**
 * render_thread() - To spawn a new threads to create a software timer interrupt to
 * be used for LED Matrix rendering
 * 
 * @args: Arguments passed to this thread, the Fallsafe context, but not actually used since the timer interrupt handler needs global variable anyway.  
*/
static void *render_thread(void *args)
{
    (void)args; // Discard unused args
    // Timer interval configurations
    struct itimerval timerval;
    // 15 Hz starting interval
    timerval.it_value.tv_sec = 0;
    timerval.it_value.tv_usec = (long long)(1000.0 / 15 * 1000);
    // 15 Hz subsequent interval
    timerval.it_interval.tv_sec = 0;
    timerval.it_interval.tv_usec = (long long)(1000.0 / 15 * 1000);

    // Configure the timer handler function
    struct sigaction action = {0};
    action.sa_handler = &timer_interupt_handler;

    // Making the timer to raise SIGALARM and use the handler function configured
    sigaction(SIGALRM, &action, NULL);

    // Start the timer
    if (setitimer(ITIMER_REAL, &timerval, NULL))
    {
        fprintf(stderr, "Failure starting itimer\n");
    }

    // While the program is continue running
    while (continueProgram)
    {
        // Pause the thread to save resources and wait for the next render tick
        pthread_cond_wait(&pthread_timer_cond, &pthread_timer_lock);
    }

    // Program is signaled to end and the thread is ended
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

    // Used to track update interval delta time
    double previousTime;
    double currentTime;

    // Initialising the Fallsafe program context
    // The dataset queue for data gathering
    context.acceleroDataset = queue_new(Vector3, queueTarget + 1);
    // The portion to be copied from the acceleroDataset for activity detection
    context.acceleroDataChunk = (Vector3 *)malloc(sizeof(Vector3) * queueTarget * 3);
    // The data to be tranformed from acceleroDataChunk to be passed to ML model for processing
    context.unrolledDataChunk = (double *)malloc(sizeof(double) * queueTarget * 3);
    // Start off as initial state to only collect data without processing as the queue as to be filled to 60
    context.state = FALLSAFE_INITIAL;
    // Poll event config joy stick
    context.evpoll.events = POLLIN;
    // Email address for Fall alert notification
    context.emailAddress = config.emailAddress;
    // Thingsboard MQTT token
    context.mqttAccessToken = config.mqttAccessToken;
    // Interval to find out what is the activity
    context.processingIntervalMS = config.processingIntervalMS;
    // Intial activity to be assumed to be stationary
    context.activityState = STATIONARY;

    // Set up program termination handler
    signal(SIGINT, exit_handler);
    // Set up wiring pi
    wiringPiSetupSys();

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
    // Set up joy stick
    if (initJoystick(&context.joystickFB) == -1)
    {
        fprintf(stderr, "[ERROR] Unable to open joystick event\n");
        return -1;
    }
    context.evpoll.fd = context.joystickFB;

    // Set up machine learning
    context.classifier = classifier_new();
    if (context.classifier == NULL)
    {
        fprintf(stderr, "[ERROR] Unable to initialize model\n");
    }

    // Start the thread managing software timer interrupt used for LED Matrix rendering
    pthread_t renderThreadId;
    if (config.enableLED)
    {
        pthread_create(&renderThreadId, NULL, render_thread, &context);
    }

    // Set up MQTT Wrapper
    mqtt_open_socket();
    mqtt_setup_client(context.mqttAccessToken);

    // Take note of the application starting time
    context.applicationStartTime = get_unixtime_ms();
    previousTime = get_monotonicclock_ms();

    // The heart beat of the program to run various taks
    while (continueProgram) // Program will end if this set to false, in exit_handler function, triggered by CTRL+C
    {
        // Pause the execution to save resources
        delayMicroseconds(updateIntervalMicroSeconds);
        // Calculates the delta time
        currentTime = get_monotonicclock_ms();
        context.deltaTime = currentTime - previousTime;
        previousTime = currentTime;
        context.unixTime = get_unixtime_ms();
        // Call the update function
        update(&context);
    }

    // Synchronize and wait for the render thread to finish
    if (config.enableLED)
    {
        pthread_join(renderThreadId, NULL);
    }
    mqtt_dispose();
    // Disposing Fallsafe context resources
    free(context.acceleroDataChunk);
    free(context.unrolledDataChunk);
    queue_destroy(context.acceleroDataset);
    classifier_destroy(context.classifier);
    setMap(0x0000, context.sensehatLEDMap, &context.sensehatfbfd);
    shShutdown(&context.sensehatfbfd, context.sensehatLEDMap);
    printf("[INFO] Program terminated\n");
    return 0;
}

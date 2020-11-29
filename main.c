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

static bool continueProgram = true;
static unsigned int updateIntervalMicroSeconds = 100;

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
        fallsafe_update_rolling_led(context);
        fallsafe_check_perform_task(context);
        break;
    case FALLSAFE_FALLEN:
        fallsafe_await_userinput(context);
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
    puts("[INFO] DEBUG MODE");
#endif // DEBUG

    Configuration config = parse_command_line(argc, argv);

    FallsafeContext context;
    double previousTime;
    double currentTime;
    int toleranceTime = 0;

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

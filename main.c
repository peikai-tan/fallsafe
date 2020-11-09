#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <wiringPi.h>
#include <sys/time.h>
#include <time.h>

#include "common/queue.h"
#include "common/vector3.h"

#include "lib/sensehat/sensehat.h"

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
static unsigned int updateIntervalMicroSeconds = 500;
static size_t queueLimit = 100;

typedef struct fallsafe_context
{
    double applicationStartTime;
    double deltaTime;
    double unixTime;
    double actualInterval;
    Queue acceleroDataset;
    int joystickFB;
    int senseHatfbfd;
    uint16_t *sensehatLEDMap;
} FallsafeContext;

/**
 * Read data from accelerometer and push to dataset queue 
*/
static Vector3 gather_data(const FallsafeContext* context)
{
    Vector3 acceleroData;
    shGet2GAccel(&acceleroData);
#if defined(DEBUG)
    printf("[Gather Data] unixtime: %lf actualinterval: %lf accelerometer: ", context->unixTime, context->deltaTime);
    vector3_print(&acceleroData);
#endif // DEBUG
    queue_enqueue(context->acceleroDataset, &acceleroData);
    return acceleroData;
}

static ActivityState process_data(ArrayList accelero_datachunk)
{
    // ML processing

    return STATIONARY;
}

static void send_thingsboardAccel(Vector3 data, double time_ms)
{
    // Send data to thingsboard
}

static void send_thingsboardState(ActivityState state, double time_ms)
{
    // Send data to thingsboard
}

static void perform_task(FallsafeContext* context)
{
    // Gather the sensor data at current moment instant
    Vector3 acceleroData = gather_data(context);

    send_thingsboardAccel(acceleroData, context->unixTime);
    // Check if queue is at the limit
    if (context->acceleroDataset->length == queueLimit)
    {
        // Dequeue buffer
        ArrayList dataChunk = arraylist_new(Vector3, queueLimit * 1.25);
        Vector3 data;
        // Dequeue all
        while (context->acceleroDataset->length)
        {
            queue_dequeue(context->acceleroDataset, &data);
            arraylist_push(dataChunk, &data);
        }
        // Pass the deqeueued chunk to ML and get the activity state
        ActivityState state = process_data(dataChunk);
        send_thingsboardState(state, context->unixTime);
        arraylist_destroy(dataChunk);
        // Show LED
        setMap(0x0000, context->sensehatLEDMap, &context->senseHatfbfd);
        drawActivity(state, context->sensehatLEDMap, &context->senseHatfbfd);
    }
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
    }
}

/**
 * Main update loop
*/
static void update(FallsafeContext* context)
{
    check_perform_task(context);
}

/**
 * Gets unix time offset in milliseconds
*/
static double get_unixtime_ms()
{
    static struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return currentTime.tv_sec * 1000.0 + currentTime.tv_usec / 1000.0;
}

/**
 * Gets high precision arbitary time in milliseconds
*/
static double get_monotonicclock_ms(void)
{
    static struct timespec _ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &_ts);
    return _ts.tv_sec * 1000.0 + _ts.tv_nsec / 1000000.0;
}

/**
 * Handles CTRL+C to gracefully exit program
*/
void exit_handler(int signum)
{
    printf("\nHandling SIGINT: %d\n", signum);
    continueProgram = false;
}

int main(int agc, char **argv)
{
    FallsafeContext context;
    double previousTime;
    double currentTime;

    context.acceleroDataset = queue_new(Vector3, queueLimit * 1.25);

    // Set up programming termination handler
    signal(SIGINT, exit_handler);
    // Set up wiring pi
    wiringPiSetupSys();

    // Set up sensehat sensors
    if (shInit(1, &context.senseHatfbfd) == 0)
    {
        fprintf(stderr, "Unable to open sense, is it connected?\n");
        return -1;
    }
    if (mapLEDFrameBuffer(&context.sensehatLEDMap, &context.senseHatfbfd) == 0)
    {
        fprintf(stderr, "Unable to map LED to Frame Buffer. \n");
        return -1;
    }
    if (initJoystick(&context.joystickFB) == -1)
    {
        fprintf(stderr, "Unable to open joystick event\n");
        return -1;
    }

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

    queue_destroy(context.acceleroDataset);
    setMap(0x0000, context.sensehatLEDMap, &context.senseHatfbfd);
    shShutdown(&context.senseHatfbfd, context.sensehatLEDMap);
    printf("Program terminated\n");
    return 0;
}

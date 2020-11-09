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
static bool continueProgram = true;
static unsigned int dataGatherIntervalMS = 16;
static unsigned int updateIntervalMicroSeconds = 500;
static size_t queueLimit = 100;

static Queue acceleroDataset;
double applicationStartTime;

// Sense hat
int senseHatfbfd = 0;
uint16_t *sensehatLEDMap = 0;

/**
 * Read data from accelerometer and push to dataset queue 
*/
static Vector3 gather_data(double actual_interval_ms, double unix_time_ms)
{
    Vector3 acceleroData;
    shGet2GAccel(&acceleroData);
#if defined(DEBUG)
    printf("[Gather Data] unixtime: %lf actualinterval: %lf accelerometer: ", unix_time_ms, actual_interval_ms);
    vector3_print(&acceleroData);
#endif // DEBUG
    queue_enqueue(acceleroDataset, &acceleroData);
    return acceleroData;
}

static ActivityState process_data(ArrayList accelero_datachunk)
{
    // ML processing

    return STATIONARY;
}

static void send_thingsboard(Vector3 data)
{
    // Send data to thingsboard
}


static void perform_task(const double *actual_interval, const double *delta_time_ms, const double *unix_time_ms)
{
    // Gather the sensor data at current moment instant
    Vector3 acceleroData = gather_data(*actual_interval, *unix_time_ms);

    send_thingsboard(acceleroData);
    // Check if queue is at the limit
    if (acceleroDataset->length == queueLimit)
    {
        // Dequeue buffer
        ArrayList dataChunk = arraylist_new(Vector3, queueLimit * 1.25);
        Vector3 data;
        // Dequeue all
        while (acceleroDataset->length)
        {
            queue_dequeue(acceleroDataset, &data);
            arraylist_push(dataChunk, &data);
        }
        // Pass the deqeueued chunk to ML and get the activity state
        ActivityState state = process_data(dataChunk);
        arraylist_destroy(dataChunk);
        // Show LED
        setMap(0x0000, sensehatLEDMap, &senseHatfbfd);
        drawActivity(state, sensehatLEDMap, &senseHatfbfd);
    }
}

/**
 * To runs every update to check if it is time to gather data and process them
*/
static void check_perform_task(const double *delta_time_ms, const double *unix_time_ms)
{
    static double timepassed = 0;
    static double previousTime = 0;
    static double actualInterval = 0;
    timepassed += *delta_time_ms;
    if (timepassed >= dataGatherIntervalMS)
    {
        previousTime = previousTime == 0 ? applicationStartTime : previousTime;
        actualInterval = *unix_time_ms - previousTime;
        previousTime = *unix_time_ms;
        perform_task(&actualInterval, delta_time_ms, unix_time_ms);
        timepassed -= dataGatherIntervalMS;
    }
}

/**
 * Main update loop
*/
static void update(double delta_time_ms, double unix_time_ms)
{
    check_perform_task(&delta_time_ms, &unix_time_ms);
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
 * Gets high previous arbitary time in milliseconds
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
    printf("Handling: %d\n", signum);
    continueProgram = false;
}

int main(int agc, char **argv)
{
    double previousTime;
    double currentTime;
    double deltaTime;

    acceleroDataset = queue_new(Vector3, 125);

    int joystickFB = 0;

    // Set up programming termination handler
    signal(SIGINT, exit_handler);
    // Set up wiring pi
    wiringPiSetupSys();

    // Set up sensehat sensors
    if (shInit(1, &senseHatfbfd) == 0)
    {
        printf("Unable to open sense, is it connected?\n");
        return -1;
    }
    if (mapLEDFrameBuffer(&sensehatLEDMap, &senseHatfbfd) == 0)
    {
        printf("Unable to map LED to Frame Buffer. \n");
        return -1;
    }
    if (initJoystick(&joystickFB) == -1)
    {
        printf("Unable to open joystick event\n");
        return -1;
    }

    applicationStartTime = get_unixtime_ms();
    previousTime = get_monotonicclock_ms();

    while (continueProgram)
    {
        delayMicroseconds(updateIntervalMicroSeconds);
        currentTime = get_monotonicclock_ms();
        deltaTime = currentTime - previousTime;
        previousTime = currentTime;
        update(deltaTime, get_unixtime_ms());
    }

    queue_destroy(acceleroDataset);
    setMap(0x0000, sensehatLEDMap, &senseHatfbfd);
    shShutdown(&senseHatfbfd, sensehatLEDMap);
    printf("Program terminated\n");
    return 0;
}

#if !defined(FALLSAFE_H)
#define FALLSAFE_H

#include <stdbool.h>

#include <poll.h>

#include "common/queue.h"
#include "common/vector3.h"

#include "utils/mqtt-sender.h"
#include "genann/combined_classifier.h"

extern const size_t queueTarget;

// Enum far various activity states
typedef enum fallsafe_state
{
    // Initial data gathering state
    FALLSAFE_INITIAL,
    // Activity detection state
    FALLSAFE_NORMAL,
    // Fallen state awaiting user input
    FALLSAFE_FALLEN
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
 * fallsafe_check_perform_task() - To runs every update to check if it is time to 
 * gather data and process them
 * @context: FallsafeContext class that contains program-wide variables
 *
 * Determine time interval for data gathering and processing
*/
void fallsafe_check_perform_task(FallsafeContext *context);

/**
 * fallsafe_await_userinput() - Triggers when falling has been detected 
 * @context: FallsafeContext class that contains program-wide variables
 *
 * If falling is detected, program state will changed to waiting 
 * user input to indicate whether it is an false positive.
 * If no input is detected after a certain period of time, 
 * an alert to external output will be triggered
*/
void fallsafe_await_userinput(FallsafeContext *context);

/**
 * fallsafe_update_rolling_led() - Set pixel for LED Array
 * @context: FallsafeContext class that contains program-wide variables
 * 
 * Spawns red-lit leds and animates it around the border of the LED matrix.
 */
void fallsafe_update_rolling_led(FallsafeContext *context);

#endif // FALLSAFE_H

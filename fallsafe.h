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

void fallsafe_check_perform_task(FallsafeContext *context);
void fallsafe_await_userinput(FallsafeContext *context);
void fallsafe_update_rolling_led(FallsafeContext *context);

#endif // FALLSAFE_H

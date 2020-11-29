#include "fallsafe.h"

// Configurations
// Define constant for frequency in Hz
#define GatheringFrequency 30
// Global variables for program-wide settings
const size_t queueTarget = 60;

static double dataGatherIntervalMS = 1000.0 / GatheringFrequency;

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
        previousState = state;

        if (state == FALLING)
        {
            context->state = FALLSAFE_FALLEN;
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
        context->state = FALLSAFE_INITIAL;
        return;
    }
    context->state = FALLSAFE_NORMAL;

    // Dequeue buffer
    Vector3 data;
    queue_dequeue(context->acceleroDataset, &data);

    // Pass data for processing
    check_process_data(context);
}

/**
 * fallsafe_check_perform_task() - To runs every update to check if it is time to 
 * gather data and process them
 * @context: FallsafeContext class that contains program-wide variables
 *
 * Determine time interval for data gathering and processing
*/
void fallsafe_check_perform_task(FallsafeContext *context)
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
void send_externalalert(FallsafeContext *context)
{
    if (context->emailAddress == NULL)
    {
        printf("[Email Sending] No Email Provided\n");
        return;
    }
    char endPoint[256] = {0};
    char outputBuffer[256] = {0};
    // Places email address into post request and store into endpoint
    sprintf(endPoint, "curl -sL https://script.google.com/macros/s/AKfycbzbGoq2XFMekzfR9OmpScNKFZ91G-CENr1Np6yWnljyPKCLjJDh/exec?email=%s", context->emailAddress);
    // Opens a process to execute curl command
    printf("[Email Sending] Recipient: %s Status: \n", context->emailAddress);
    FILE *curlProcess = popen(endPoint, "w");
    // Reads http response
    while (fgets(outputBuffer, 255, curlProcess) != NULL)
        printf("%s", outputBuffer);

    // Close process
    pclose(curlProcess);
    puts("\nEnd Request");
}

/**
 * fallsafe_await_userinput() - Triggers when falling has been detected 
 * @context: FallsafeContext class that contains program-wide variables
 *
 * If falling is detected, program state will changed to waiting 
 * user input to indicate whether it is an false positive.
 * If no input is detected after a certain period of time, 
 * an alert to external output will be triggered
*/
void fallsafe_await_userinput(FallsafeContext *context)
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
                    context->state = FALLSAFE_INITIAL;
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

                // Selects next option
                case UP:
                case RIGHT:
                    currentSelection = (currentSelection + 1) % 4;
                    context->activityState = selections[currentSelection];
                    break;

                // Selects previous option
                case DOWN:
                case LEFT:
                    currentSelection = currentSelection - 1 < 0 ? 3 : (currentSelection - 1);
                    context->activityState = selections[currentSelection];
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
 * fallsafe_update_rolling_led() - Set pixel for LED Array
 * @context: FallsafeContext class that contains program-wide variables
 * 
 * Spawns red-lit leds and animates it around the border of the LED matrix.
 */
void fallsafe_update_rolling_led(FallsafeContext *context)
{
    static int previousPosition = 0;
    static int currentPosition = 0;
    // Top row
    shSetPixel(previousPosition, 0, 0, 1, context->sensehatLEDMap, &context->sensehatfbfd);
    shSetPixel(currentPosition, 0, 0x4000, 1, context->sensehatLEDMap, &context->sensehatfbfd);
    // Right column
    shSetPixel(7, previousPosition, 0, 1, context->sensehatLEDMap, &context->sensehatfbfd);
    shSetPixel(7, currentPosition, 0x4000, 1, context->sensehatLEDMap, &context->sensehatfbfd);
    // Left column
    shSetPixel(0, 7 - previousPosition, 0, 1, context->sensehatLEDMap, &context->sensehatfbfd);
    shSetPixel(0, 7 - currentPosition, 0x4000, 1, context->sensehatLEDMap, &context->sensehatfbfd);
    // Botton row
    shSetPixel(7 - previousPosition, 7, 0, 1, context->sensehatLEDMap, &context->sensehatfbfd);
    shSetPixel(7 - currentPosition, 7, 0x4000, 1, context->sensehatLEDMap, &context->sensehatfbfd);
    previousPosition = currentPosition;
    currentPosition = (currentPosition + 1) % 7;
}

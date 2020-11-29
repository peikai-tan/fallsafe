// This file contains the function definition for setting up and
// sending data to MQTT.
//
// The file is a rewritten version of examples provided in MQTT-C repository.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "posix_sockets.h"
#include "mqtt-sender.h"
/**
 * Global variables for
 * Thingsboard properties such as the address, port, access token
 * and topic to publish the data to.
 * MQTT client object and client Id for easier reference
 * Sending and Receiving Buffers
 * Connection flags
 * Client Daemon for multi-threading
 * File descriptor for sockets
 */
static const char *addr = "129.126.163.157";
static const char *port = "1883";
static const char *topic = "v1/devices/me/telemetry";
static char *accessToken = NULL;
static struct mqtt_client mqttClient;
static uint8_t sendBuf[10240];
static uint8_t receiveBuf[1024];
static char *clientId = NULL;
static uint8_t connectFlags = MQTT_CONNECT_CLEAN_SESSION;
static pthread_t clientDaemon;
static int sockfd;
static volatile bool isRunning = true;

/**
 * mqtt_exit - To properly close of the MQTT client when error occurs
 * @status: The exit code
 * @sockfd: File descriptor of the socket
 * @client_daemon: Pointer to the thread of the client daemon
 *
 * This function is to properly close the MQTT client to avoid memory
 * leaks potentially caused by the threads when error occurs
 */
void mqtt_exit(int status, int sockfd, pthread_t *client_daemon)
{
    if (client_daemon) //If daemon is still running
    {
        isRunning = false; // Set the state to not running & join threads.
        pthread_join(*client_daemon, NULL);
    }
    if (sockfd != -1) // Socket is not closed, close it.
    {
        close(sockfd);
    }
    exit(status); // Exit with provided status
}

/**
 * mqtt_dispose - To properly close of the MQTT client when no error occurs at the
 * end program
 *
 * This function is to properly close the MQTT client to avoid memory
 * leaks potentially caused by the threads. However, this function differs
 * from mqtt_exit as this function is meant for non-error exits.
 */
void mqtt_dispose(void)
{
    if (accessToken == NULL)
    {
        return;
    }
    isRunning = false;
    pthread_join(clientDaemon, NULL);
    if (sockfd != -1)
    {
        close(sockfd);
    }
}

/**
 * client_refresher() - Wait for half a second before sending the traffic over
 * @client: Pointer to MQTT client
 *
 * This function is to send the traffic within the client object every 0.5 seconds
 */
void *client_refresher(void *args)
{
    struct mqtt_client *client = (struct mqtt_client *)args;
    while (isRunning)
    {
        mqtt_sync(client);
        usleep(500000U);
    }
    return NULL;
}

/**
 * publish_callback() - Prints the Publish Response
 * @unsued: Reserved for future use
 * @client: Pointer to MQTT Publish Response
 *
 * This function is to print the MQTT publish response
 */
void publish_callback(void **unused, struct mqtt_response_publish *published)
{
    (void)unused;
    printf("[MQTT] Publish response: %s", (char *)published->application_message);
}

/**
 * mqtt_open_socket() - Open the sockets for MQTT
 *
 * Based on the address and port provided in global variable, open socket.
 */
void mqtt_open_socket(void)
{
    sockfd = open_nb_socket(addr, port);
    if (sockfd == -1)
    {
        perror("Failed to open socket: ");
        mqtt_exit(EXIT_FAILURE, sockfd, NULL);
    }
}

/**
 * mqtt_set_token() - Set the access token
 * @access_token: Read-only string of access token to replace
 * Replace the default global variable token with provided.
 *
 * Return: Success value from setting token
 * 1 - Access token successfully replaced without error.
 * 0 - Error occured.
 */
int mqtt_set_token(const char *access_token)
{
    if ((snprintf(accessToken, strlen(accessToken), "%s", access_token)) < 1)
        return 0;
    else
        return 1;
}

/**
 * mqtt_setup_client() - Setup the MQTT client
 * @access_token: Read-only string of access token to replace
 *
 * Replace the global variable access token, initialise the mqtt client with the socket and buffer.
 * Establish connection the MQTT.
 * If error occured, exit with mqtt_exit to close client properly.
 */
void mqtt_setup_client(char *access_token)
{
    accessToken = access_token;
    if (accessToken == NULL)
    {
        printf("[MQTT] NOT ENABLED, no access token provided\n");
        return;
    }
    mqtt_init(&mqttClient, sockfd, sendBuf, sizeof(sendBuf), receiveBuf, sizeof(receiveBuf), publish_callback);
    mqtt_connect(&mqttClient, clientId, NULL, NULL, 0, accessToken, NULL, connectFlags, 30);
    if (mqttClient.error != MQTT_OK)
    {
        fprintf(stderr, "Error: %s\n", mqtt_error_str(mqttClient.error));
        mqtt_exit(EXIT_FAILURE, sockfd, NULL);
    }
    if (pthread_create(&clientDaemon, NULL, client_refresher, &mqttClient))
    {
        fprintf(stderr, "Failed to start client daemon. \n");
        mqtt_exit(EXIT_FAILURE, sockfd, NULL);
    }
}

/**
 * mqtt_send_vector3() - Send the vector3 data to Thingsboard
 * @vector: Pointer to the vector 3 struct
 * @time_ms: Time stamp of data (unused)
 *
 * Format the vector3 data to json format required by Thingsboard and publish to Thingsboard.
 * If error occured during sending, exit with mqtt_exit().
 * time_ms is not used due to Thingsboard is ahead of time by few minutes, providing the time will mess up
 * the dashboard.
 *
 */
void mqtt_send_vector3(Vector3 *vector, long long time_ms)
{
    (void)time_ms; // Not using as the server time actually around 2 mins ahead of standard time
    if (accessToken == NULL)
    {
        return;
    }
    char application_message[256] = {0};
    snprintf(application_message, sizeof(application_message), "{\"x\": %lf, \"y\": %lf, \"z\": %lf}", vector->x, vector->y, vector->z);
    mqtt_publish(&mqttClient, topic, application_message, strlen(application_message), MQTT_PUBLISH_QOS_0);
    if (mqttClient.error != MQTT_OK)
    {
        fprintf(stderr, "Error: %s\n", mqtt_error_str(mqttClient.error));
        mqtt_exit(EXIT_FAILURE, sockfd, &clientDaemon);
    }
}

/**
 * mqtt_send_activity() - Send the Activity State to Thingsboard
 * @activity_state: Activity State enumerator
 * @time_ms: Time stamp of data (unused)
 *
 * Format the enumerator to json format required by Thingsboard and publish to Thingsboard.
 * If error occured during sending, exit with mqtt_exit().
 * time_ms is not used due to Thingsboard is ahead of time by few minutes, providing the time will mess up
 * the dashboard.
 *
 */
void mqtt_send_activity(ActivityState activity_state, long long time_ms)
{
    (void)time_ms; // Not using as the server time actually around 2 mins ahead of standard time
    if (accessToken == NULL)
    {
        return;
    }
    char application_message[256] = {0};
    switch (activity_state)
    {
    case (FALLING):
        snprintf(application_message, sizeof(application_message), "{\"Activity State\": \"%s\"}", "FALLING");
        break;
    case (WALKING):
        snprintf(application_message, sizeof(application_message), "{\"Activity State\": \"%s\"}", "WALKING");
        break;
    case (RUNNING):
        snprintf(application_message, sizeof(application_message), "{\"Activity State\": \"%s\"}", "RUNNING");
        break;
    case (JUMPING):
        snprintf(application_message, sizeof(application_message), "{\"Activity State\": \"%s\"}", "JUMPING");
        break;
    case (STATIONARY):
        snprintf(application_message, sizeof(application_message), "{\"Activity State\": \"%s\"}", "STATIONARY");
        break;
    default:
        break;
    }
    if (strlen(application_message) > 0)
    {
        mqtt_publish(&mqttClient, topic, application_message, strlen(application_message), MQTT_PUBLISH_QOS_0);
        //printf("Published: \n%s\n", application_message);
        if (mqttClient.error != MQTT_OK)
        {
            fprintf(stderr, "Error: %s\n", mqtt_error_str(mqttClient.error));
            //mqtt_exit(EXIT_FAILURE, sockfd, &clientDaemon);
        }
    }
}

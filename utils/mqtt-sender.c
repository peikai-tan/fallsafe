#include <stdio.h>
#include "posix_sockets.h"
#include "mqtt-sender.h"
#include <unistd.h>
#include <stdlib.h>

static const char *addr = "129.126.163.157";
static const char *port = "1883";
static const char *topic = "v1/devices/me/telemetry";
static char *accessToken = "5NpDeYwlNOwpLbhtPy1V";
static struct mqtt_client mqttClient;
static uint8_t sendBuf[2048];
static uint8_t receiveBuf[1024];
static char *clientId = NULL;
static uint8_t connectFlags = MQTT_CONNECT_CLEAN_SESSION;
static pthread_t clientDaemon;
static int sockfd;

void mqtt_exit(int status, int sockfd, pthread_t *client_daemon)
{
    if (sockfd != -1)
        close(sockfd);
    if (client_daemon)
        pthread_cancel(*client_daemon);
    exit(status);
}

void *client_refresher(void *client)
{
    while (1)
    {
        mqtt_sync((struct mqtt_client *)client);
        usleep(1000000U);
    }
    return NULL;
}

void publish_callback(void **unused, struct mqtt_response_publish *published)
{
    printf("Publish response: %s", (char *)published->application_message);
}

void mqtt_open_socket(void)
{
    sockfd = open_nb_socket(addr, port);
    if (sockfd == -1)
    {
        perror("Failed to open socket: ");
        mqtt_exit(EXIT_FAILURE, sockfd, NULL);
    }
}

int mqtt_set_token(const char *access_token)
{
    if ((snprintf(accessToken, strlen(accessToken), "%s", access_token)) < 1)
        return 0;
    else
        return 1;
}

void mqtt_setup_client(void)
{
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

void mqtt_send_vector3(Vector3 *vector, long long time_ms)
{
    char application_message[256];
    snprintf(application_message, sizeof(application_message), "{\"ts\": %lld,\"values\": {\"x\": %lf, \"y\": %lf, \"z\": %lf}}", time_ms, vector->x, vector->y, vector->z);
    mqtt_publish(&mqttClient, topic, application_message, strlen(application_message), MQTT_PUBLISH_QOS_0);
    printf("Published: \n%s\n", application_message);
    if (mqttClient.error != MQTT_OK)
    {
        fprintf(stderr, "Error: %s\n", mqtt_error_str(mqttClient.error));
        mqtt_exit(EXIT_FAILURE, sockfd, &clientDaemon);
    }
    else
        puts("No Errors");
}

void mqtt_send_activity(ActivityState activity_state, long long time_ms)
{
    char application_message[256] = {0};
    switch (activity_state)
    {
    case (FALLING):
        snprintf(application_message, sizeof(application_message), "{\"ts\": %lld, \"values\": {\"Activity State\": %s}}", time_ms, "FALLING");
        break;
    case (WALKING):
        snprintf(application_message, sizeof(application_message), "{\"ts\": %lld, \"values\": {\"Activity State\": %s}}", time_ms, "WALKING");
        break;
    case (RUNNING):
        snprintf(application_message, sizeof(application_message), "{\"ts\": %lld, \"values\": {\"Activity State\": %s}}", time_ms, "RUNNING");
        break;
    case (JUMPING):
        snprintf(application_message, sizeof(application_message), "{\"ts\": %lld, \"values\": {\"Activity State\": %s}}", time_ms, "JUMPING");
        break;
    case (STATIONARY):
        snprintf(application_message, sizeof(application_message), "{\"ts\": %lld, \"values\": {\"Activity State\": %s}}", time_ms, "STATIONARY");
        break;
    default:
        break;
    }
    if (strlen(application_message) > 0)
    {
        mqtt_publish(&mqttClient, topic, application_message, strlen(application_message), MQTT_PUBLISH_QOS_0);
        printf("Published: \n%s\n", application_message);
        if (mqttClient.error != MQTT_OK)
        {
            fprintf(stderr, "Error: %s\n", mqtt_error_str(mqttClient.error));
            mqtt_exit(EXIT_FAILURE, sockfd, &clientDaemon);
        }
        else
            puts("No Errors");
    }
}

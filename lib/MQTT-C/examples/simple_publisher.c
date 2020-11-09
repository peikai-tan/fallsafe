#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <mqtt.h>
#include "templates/posix_sockets.h"

void exit_example(int status, int sockfd, pthread_t *client_daemon)
{
    if (sockfd != -1) close(sockfd);
    if (client_daemon != NULL) pthread_cancel(*client_daemon);
    exit(status);
}

void publish_callback(void** unused, struct mqtt_response_publish *published) 
{
    printf("publish response: %s", (char*)published->application_message);
}

void* client_refresher(void* client)
{
    while(1) 
    {
        //puts("Refreshing client");
        mqtt_sync((struct mqtt_client*) client);
        usleep(1000000U);
    }
    return NULL;
}

int main(void) 
{
    const char* addr = "129.126.163.157";
    const char* port = "1883";
    const char* topic = "v1/devices/me/telemetry";
    const char* accessToken = "5NpDeYwlNOwpLbhtPy1V";

    /* open the non-blocking TCP socket (connecting to the broker) */
    int sockfd = open_nb_socket(addr, port);

    if (sockfd == -1) {
        perror("Failed to open socket: ");
        exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    /* setup a client */
    struct mqtt_client client;
    uint8_t sendbuf[2048]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
    uint8_t recvbuf[1024]; /* recvbuf should be large enough any whole mqtt message expected to be received */
    mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback);
    /* Create an anonymous session */
    const char* client_id = NULL;
    /* Ensure we have a clean session */
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    /* Send connection request to the broker. */
    mqtt_connect(&client, client_id, NULL, NULL, 0, accessToken, NULL, connect_flags, 30);

    /* check that we don't have any errors */
    if (client.error != MQTT_OK) {
        fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
        exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    /* start a thread to refresh the client (handle egress and ingree client traffic) */
    pthread_t client_daemon;
    if (pthread_create(&client_daemon, NULL, client_refresher, &client)) {
        fprintf(stderr, "Failed to start client daemon.\n");
        exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    /* start publishing the values */
    printf("Press CTRL-D (or any other key) to exit.\n\n");
    printf("Press ENTER to publish the values.\n");
    while(fgetc(stdin) == '\n') {
        /* print a message */
        char application_message[256];
        snprintf(application_message, sizeof(application_message), "{\"%s\": %d}", "random", rand());

        /* publish the time */
        mqtt_publish(&client, topic, application_message, strlen(application_message), MQTT_PUBLISH_QOS_0);
        printf("published : \n%s\n", application_message);


        /* check for errors */
        if (client.error != MQTT_OK) {
            fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
            exit_example(EXIT_FAILURE, sockfd, &client_daemon);
        }
        else {
            puts("No Errors");
        }
    }   

    /* disconnect */
    printf("\ndisconnecting from %s\n", addr);
    sleep(1);

    /* exit */ 
    exit_example(EXIT_SUCCESS, sockfd, &client_daemon);
}


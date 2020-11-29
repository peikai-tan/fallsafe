// This file contains the function declaration for setting up and
// sending data to MQTT.
//
// The file is a rewritten version of examples provided in MQTT-C repository.
//
#include "../common/vector3.h"
#include "../lib/sensehat/sensehat.h"

#include <pthread.h>
#include <mqtt.h>

/**
 * mqtt_set_token() - Set the access token
 * @access_token: Read-only string of access token to replace
 * Replace the default global variable token with provided.
 *
 * Return: Success value from setting token
 * 1 - Access token successfully replaced without error.
 * 0 - Error occured.
 */
int mqtt_set_token(const char *access_token);

/**
 * mqtt_exit - To properly close of the MQTT client when error occurs
 * @status: The exit code
 * @sockfd: File descriptor of the socket
 * @client_daemon: Pointer to the thread of the client daemon
 *
 * This function is to properly close the MQTT client to avoid memory
 * leaks potentially caused by the threads when error occurs
 */
void mqtt_exit(int status, int sockfd, pthread_t *client_daemon);

/**
 * client_refresher() - Wait for half a second before sending the traffic over
 * @client: Pointer to MQTT client
 *
 * This function is to send the traffic within the client object every 0.5 seconds
 */
void *client_referesher(void *client);

/**
 * publish_callback() - Prints the Publish Response
 * @unsued: Reserved for future use
 * @client: Pointer to MQTT Publish Response
 *
 * This function is to print the MQTT publish response
 */
void publish_callback(void **unused, struct mqtt_response_publish *published);

/**
 * mqtt_open_socket() - Open the sockets for MQTT
 *
 * Based on the address and port provided in global variable, open socket.
 */
void mqtt_open_socket(void);

/**
 * mqtt_setup_client() - Setup the MQTT client
 * @access_token: Read-only string of access token to replace
 *
 * Replace the global variable access token, initialise the mqtt client with the socket and buffer.
 * Establish connection the MQTT.
 * If error occured, exit with mqtt_exit to close client properly.
 */
void mqtt_setup_client(char *access_token);

/**
 * mqtt_dispose - To properly close of the MQTT client when no error occurs at the
 * end program
 *
 * This function is to properly close the MQTT client to avoid memory
 * leaks potentially caused by the threads. However, this function differs
 * from mqtt_exit as this function is meant for non-error exits.
 */
void mqtt_dispose(void);

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
void mqtt_send_vector3(Vector3 *vector, long long time_ms);

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
void mqtt_send_activity(ActivityState activity_state, long long time_ms);

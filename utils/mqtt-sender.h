#include "../common/vector3.h"
#include "../lib/sensehat/sensehat.h"

#include <pthread.h>
#include <mqtt.h>

int mqtt_set_token(const char * access_token);

void mqtt_exit(int status, int sockfd, pthread_t * client_daemon);

void * client_referesher(void * client);

void publish_callback(void ** unused, struct mqtt_response_publish * published);

void mqtt_open_socket(void);

void mqtt_setup_client(void);

void mqtt_send_vector3(Vector3 * vector, long long time_ms);

void mqtt_send_activity(ActivityState activity_state, long long time_ms);

#if !defined(CONFIGURATION_H)
#define CONFIGURATION_H

#include <stdbool.h>

typedef struct configuration
{
    char *mqttAccessToken;
    char *emailAddress;
    bool enableLED;
    double processingIntervalMS;
} Configuration;

Configuration parse_command_line(int argc, char **argv);

#endif // CONFIGURATION_H

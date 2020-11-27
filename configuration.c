#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "configuration.h"

static char *DefaultMQTTAccessToken = "dAWY0dhv2dx4LM1PE4sg";

/* Process command line arguments to configure the program configurations if there are any */
struct configuration
parse_command_line(int argc, char **argv)
{
    struct configuration result;
    result.emailAddress = NULL;
    result.mqttAccessToken = DefaultMQTTAccessToken;
    result.enableLED = true;
    result.processingIntervalMS = 1000.0;

    int opt;
    while ((opt = getopt(argc, argv, ":e:na:l:t:h")) != -1)
    {
        switch (opt)
        {
        case 'e':
            result.emailAddress = optarg;
            break;
        case 'a':
            result.mqttAccessToken = optarg;
            break;
        case 'n':
            result.mqttAccessToken = NULL;
            break;
        case 'l':
            result.enableLED = atoi(optarg) == 0 ? false : true;
            break;
        case 't':
            result.processingIntervalMS = strtod(optarg, NULL);
            break;
        case 'h': /* Show a simple help message showing these configurable options */
                  //TODO
            printf("TODO: help message\n");
            exit(0);
            break;
        case '?':
            printf("Unknown option -%c.\n", optopt);
            break;
        }
    }
    char *tokenType = result.mqttAccessToken == DefaultMQTTAccessToken 
        ? "(Using Team 18 Default Token)" 
        : "";
    printf("[Configuration] Email: %s\n", result.emailAddress);
    printf("[Configuration] MQTT Token: %s %s\n", result.mqttAccessToken, tokenType);
    printf("[Configuration] LED: %d\n", result.enableLED);
    printf("[Configuration] Interval: %lf ms\n", result.processingIntervalMS);
    return result;
}
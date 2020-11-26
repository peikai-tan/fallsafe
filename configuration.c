#include <unistd.h>
#include <stdlib.h>
#include "configuration.h"

/* Process command line arguments to configure the program configurations if there are any */
struct configuration parse_command_line(int argc, char **argv)
{
    struct configuration result;
    result.emailAddress = NULL;
    result.mqttAccessToken = NULL;
    result.enableLED = true;
    result.processingIntervalMS = 1000.0 / 30;

    int opt;
    while ((opt = getopt(argc, argv, ":e:a:l:t:h")) != -1)
    {
        switch (opt)
        {
        case 'e':
            result.emailAddress = optarg;
            break;
        case 'a':
            result.mqttAccessToken = optarg;
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
    printf("Email: %s\n", result.emailAddress);
    printf("Token: %s\n", result.mqttAccessToken);
    printf("LED: %d\n", result.enableLED);
    printf("Interval: %lf ms\n", result.processingIntervalMS);
    return result;
}
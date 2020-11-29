#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "configuration.h"

static char *DefaultMQTTAccessToken = "dAWY0dhv2dx4LM1PE4sg";

/* Process command line arguments to configure the program configurations if there are any */
Configuration parse_command_line(int argc, char **argv)
{
    Configuration result;
    /* Email Address set to NULL when user did not set Email Address for the Fall Alert */
    result.emailAddress = NULL;
    /* Using our team's Thingsboard Access Token */
    result.mqttAccessToken = DefaultMQTTAccessToken;
    /* Default LED Usage enabled */
    result.enableLED = true;
    /* Default Data Processing Frequency */
    result.processingIntervalMS = 500.0;

    /* Loops through the Command Line Options entered by user and execute it accordingly */
    int opt;
    while ((opt = getopt(argc, argv, ":e:na:l:t:h")) != -1)
    {
        switch (opt)
        {
        case 'e': /* Set Email Address */
            result.emailAddress = optarg;
            break;
        case 'a': /* Set MQTT Access Token */
            result.mqttAccessToken = optarg;
            break;
        case 'n': /* To Disable MQTT by setting MQTT Access Token to NULL */
            result.mqttAccessToken = NULL;
            break;
        case 'l': /* Enable/Disable LED, by default is Enable */
            result.enableLED = atoi(optarg) == 0 ? false : true;
            break;
        case 't': /* Set Data Processing Frequency */
            result.processingIntervalMS = strtod(optarg, NULL);
            break;
        case 'h': /* Show a simple help message showing these configurable options */
            puts("-e Email recipant for the fall alert, no email sending if not supplied\n"
                 "-a MQTT Thingsboard access token, default to our teams's Thingsboard if not supplied\n"
                 "-n Disables MQTT Thingsboard\n"
                 "-t Data processing interval in milliseconds, defaults to 500\n"
                 "-l 0 to disable LED to save power, defaults to 1\n"
                 "-h Prints this help message");
            /* Exits the whole program since it is only printing the Help Message */
            exit(0);
            break;
        case '?': /* Invalid Command Line Option entered */
            printf("Unknown option -%c.\n", optopt);
            break;
        }
    }
    /* If the user entered the default MQTT Access Token, the system will prompt the user
        that the MQTT Access Token used is the default Token */
    char *tokenType = result.mqttAccessToken == DefaultMQTTAccessToken
                          ? "(Using Team 18 Default Token)"
                          : "";
    /* Prints out the Emaill Address, MQTT Access Token, 
        Current Token Type (Default or !Default [Empty String]),
        Enabled/Disabled LED and the Data Processing Interval in miliseconds */
    printf("[Configuration] Email: %s\n", result.emailAddress);
    printf("[Configuration] MQTT Token: %s %s\n", result.mqttAccessToken, tokenType);
    printf("[Configuration] LED: %d\n", result.enableLED);
    printf("[Configuration] Interval: %lf ms\n", result.processingIntervalMS);

    /* Returns the configuration struct, result */
    return result;
}
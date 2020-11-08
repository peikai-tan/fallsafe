#include <stdio.h>
#include <stdbool.h>
#include <wiringPi.h>

/* Psuedo code:
 *
 * Check timer thingy
 * If timer triggers, read data
    * If queue is full, dequeue
    * Then, queue data 
 * If queue is pull, push data into model, get result
 * If result is fall, wait for user input, blink red 
 * Else, continue running
 */

static bool continueProgram = true;

int main(int agc, char **argv)
{
    wiringPiSetupSys();

    while (continueProgram)
    {
        delay(1000);
    }

    printf("Program finished\n");
    return 0;
}

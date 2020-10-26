#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <locale.h>

#define INTERVAL_MS 250

static volatile bool running = true;
static volatile unsigned long long output = 0;
static volatile unsigned long long count = 0;

void timer_handler(int signum)
{
    printf("Timer event raised %llu times Speed: %'llu/s loops\n", ++count, output * (1000 / INTERVAL_MS));
    output = 0;
}

void exit_handler(int signum)
{
    printf("Handling: %d\n", signum);
    running = false;
}

int main()
{
    setlocale(LC_NUMERIC, "");
    signal(SIGINT, exit_handler);
    struct sigaction action;
    struct itimerval timespan;

    /* Install timer_handler as the signal handler for SIGVTALRM. */
    memset(&action, 0, sizeof(action));
    action.sa_handler = &timer_handler;
    sigaction(SIGVTALRM, &action, NULL);

    /* Configure the timer to expire after 250 ms... */
    timespan.it_value.tv_sec = 0;
    timespan.it_value.tv_usec = INTERVAL_MS * 1000;
    /* ... and every 250 ms after that. */
    timespan.it_interval.tv_sec = 0;
    timespan.it_interval.tv_usec = INTERVAL_MS * 1000;

    if (setitimer(ITIMER_VIRTUAL, &timespan, NULL))
    {
        fprintf(stderr, "Failure starting itimer\n");
    }

    struct timespec sleepTime;
    sleepTime.tv_nsec = 10000000;
    sleepTime.tv_sec = 0;

    /* Do busy work. */
    while (running)
    {
        ++output;
    }

    printf("Last Result: %'zu\n", output);
}
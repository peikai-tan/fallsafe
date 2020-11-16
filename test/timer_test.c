#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <unistd.h>

#include <sys/time.h>

#define INTERVAL_MS 250

static volatile bool running = true;
static volatile unsigned long long output = 0;
static volatile unsigned long long count = 0;
static volatile double previousTime = 0.0;

static double getClock(void)
{
    static struct timespec _ts;
    clock_gettime(CLOCK_MONOTONIC, &_ts);
    return _ts.tv_sec * 1000.0 + _ts.tv_nsec / 1000000.0;
}

void timer_handler(int signum)
{
    double currentTime = getClock();
    double deltaTime = currentTime - previousTime;
    previousTime = currentTime;
    printf("Timer event raised %llu times Speed: %'llu/s loops Delta Time: %lf\n", ++count, (long long)(output * (1000.0 / INTERVAL_MS)), deltaTime);
    output = 0;
}

void exit_handler(int signum)
{
    printf("Handling: %d\n", signum);
    running = false;
}

int main(void)
{
    setlocale(LC_NUMERIC, "");
    signal(SIGINT, exit_handler);
    struct sigaction action;
    struct itimerval timespan;

    /* Install timer_handler as the signal handler for SIGVTALRM. */
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = &timer_handler;
    sigaction(SIGVTALRM, &action, NULL);

    // 5Hz Timer
    // Configure the timer to expire after 250 ms
    timespan.it_value.tv_sec = INTERVAL_MS / 1000;
    timespan.it_value.tv_usec = ((long long)(INTERVAL_MS * 1000ll) % 1000000ll);
    // and every 250 ms after that
    timespan.it_interval.tv_sec = INTERVAL_MS / 1000;
    timespan.it_interval.tv_usec = ((long long)(INTERVAL_MS * 1000ll) % 1000000ll);

    if (setitimer(ITIMER_VIRTUAL, &timespan, NULL))
    {
        fprintf(stderr, "Failure starting itimer\n");
    }

    previousTime = getClock();

    /* Do busy work. */
    while (running)
    {
        ++output;
    }

    printf("Last Result: %'zu\n", (long long)output);
}
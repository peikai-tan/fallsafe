#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>

#include <pthread.h>
#include <unistd.h>
#include <locale.h>

#if !defined(__WIN32)
#include <sys/sysinfo.h>
#define COUNT_FORMATTER "%'lld"
#else
#define COUNT_FORMATTER "%lld"
#endif //

static int NO_OF_THREADS = 1;
static long long *accumulates;
static volatile bool running = true;
static volatile bool collecting = false;

void exit_handler(int signum)
{
    printf("\nHandling: %d\n", signum);
    running = false;
}

void *spin_thread(void *args)
{
    long long *input = (long long *)args;

    while (running)
    {
        if (!collecting)
        {
            *input = (*input) + 1;
        }
    }

    return NULL;
}

void reset_accumuates(void)
{
    for (int i = 0; i < NO_OF_THREADS; i++)
    {
        accumulates[i] = 0;
    }
}

long long sum(long long input[], size_t length)
{
    long long output = 0;
    for (size_t i = 0; i < length; i++)
    {
        output += accumulates[i];
    }
    return output;
}

void print_cpu_count(void)
{
#if !defined(__WIN32)
    printf("This system has %d processors configured and %d processors available.\n",
           get_nprocs_conf(),
           get_nprocs());
#endif
}

int main(int argc, char **argv)
{
    setlocale(LC_NUMERIC, "");
    signal(SIGINT, exit_handler);

    if (argc == 2)
    {
        NO_OF_THREADS = atoi(argv[1]);
    }
    long long accumulatesInit[NO_OF_THREADS];
    accumulates = accumulatesInit;

    print_cpu_count();

    printf("No of Threads: %d\n", NO_OF_THREADS);

    reset_accumuates();

    struct timespec nanosleepSpec;
    nanosleepSpec.tv_sec = 1;
    nanosleepSpec.tv_nsec = 1000000000 / 2 * 0;

    pthread_t thread_id[NO_OF_THREADS];

    for (int i = 0; i < NO_OF_THREADS; i++)
    {
        pthread_create(&thread_id[i], NULL, spin_thread, &accumulates[i]);
        printf("Created thread id %zu\n", (size_t)thread_id[i]);
    }

    while (running)
    {
        nanosleep(&nanosleepSpec, &nanosleepSpec);
        collecting = true;
        printf("[Collection Interval] " COUNT_FORMATTER " thread loops -", sum(accumulates, NO_OF_THREADS));
        for (int i = 0; i < NO_OF_THREADS; i++)
        {
            printf(" %d: " COUNT_FORMATTER " ", i, accumulates[i]);
        }
        reset_accumuates();
        puts("");
        collecting = false;
    }

    for (int i = 0; i < NO_OF_THREADS; i++)
    {
        pthread_join(thread_id[i], NULL);
        printf("Exited thread id %zu\n", (size_t)thread_id[i]);
    }

    printf("Done\n");
    return 0;
}

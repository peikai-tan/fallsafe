#include "timer.h"

void startTimer()
{
    start_t = clock();
}

void endTimer()
{
    end_t = clock();
}


double calculateTimer()
{
    double val = (double)(end_t - start_t) / CLOCKS_PER_SEC;
    return val;
}

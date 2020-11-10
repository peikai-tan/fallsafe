#include "timer.h"
#include "../time.h"

void startTimer()
{
    start_t = get_monotonicclock_ms();
}

void endTimer()
{
    end_t = get_monotonicclock_ms();
}


double calculateTimer()
{
    double val = (end_t - start_t) * 1000;
    return val;
}

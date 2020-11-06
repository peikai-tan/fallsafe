#ifndef TIMER_H
#define TIMER_H

#include <time.h>

static clock_t start_t, end_t;

void startTimer();

void endTimer();

double calculateTimer();

#endif


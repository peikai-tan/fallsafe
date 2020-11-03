#include <stdbool.h>
#include <math.h>

#include "./math.h"

bool fuzzyEqual(const float a, const float b, const float epsilon)
{
    return fabs(a - b) <= epsilon;
}

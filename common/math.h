#if !defined(COMMON_MATHS)
#define COMMON_MATHS

#include <math.h>
#include <stdbool.h>

#define clamp(input, min, max) \
    (input < min ? min : input > max ? max : input)

bool fuzzyEqual(const float a, const float b, const float epsilon);

#endif // COMMON_MATHS

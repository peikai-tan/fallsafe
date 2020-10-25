#include <string.h>

#include "memoryUtils.h"

// https://stackoverflow.com/questions/8166502/c-fastest-method-to-swap-two-memory-blocks-of-equal-size
#define SWAP_USE_WORD
#ifdef SWAP_USE_LIB
// Benchmark reference time: 0.70
void swap(void *const a, void *const b, const size_t size)
{
    void *temp = malloc(size);
    memcpy(temp, a, size);
    memcpy(b, a, size);
    memcpy(a, temp, size);
    free(temp);
}
#elif defined(SWAP_USE_VLA)
// https://stackoverflow.com/questions/57009281/is-there-is-a-problem-with-using-a-variable-length-array-in-c-for-embedded-syste
void swap(void *const a, void *const b, const size_t size)
{
    char buffer[size];
    memcpy(buffer, a, size);
    memcpy(a, b, size);
    memcpy(b, buffer, size);
}
#elif defined(SWAP_USE_LOOP)
// Benchmark reference time: 0.35
void swap(void *const a, void *const b, const size_t size)
{
    unsigned char *p;
    unsigned char *q;
    unsigned char *const count = (unsigned char *)a + size;

    for (p = a, q = b; p < count; ++p, ++q)
    {
        const unsigned char temp = *p;
        *p = *q;
        *q = temp;
    }
}
#elif defined(SWAP_USE_WORD)
// Benchmark reference time: 0.29
void swap(void *const a, void *const b, const size_t size)
{
    size_t words = size / sizeof(long);
    size_t bytes = size % sizeof(long);

    char *x = (char *)a;
    char *y = (char *)b;
    long t[1];

    while (words--)
    {
        memcpy(t, x, sizeof(long));
        memcpy(x, y, sizeof(long));
        memcpy(y, t, sizeof(long));
        x += sizeof(long);
        y += sizeof(long);
    }

    if (bytes == 0)
    {
        return;
    }

    x = (char *)a;
    y = (char *)b;

    while (bytes--)
    {
        char t = *x;
        *x = *y;
        *y = t;
        x += 1;
        y += 1;
    }
}
#endif

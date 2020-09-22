#if !defined(BIT_ROTATION_H)
#define BIT_ROTATION_H

#include <limits.h>

#define int_size (sizeof(int) * CHAR_BIT)

unsigned int rotateLeft(unsigned int intput, unsigned int times);
unsigned int rotateRight(unsigned int intput, unsigned int times);

#endif // BIT_ROTATION_H

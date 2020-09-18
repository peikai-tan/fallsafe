#include <limits.h>

#include "bit_rotation.h"

unsigned int rotateLeft(unsigned int input, unsigned int times)
{
    return (input << (times % (sizeof(input) * CHAR_BIT))) | (input >> ((sizeof(input) * CHAR_BIT) - (times % (sizeof(input) * CHAR_BIT))));
}

unsigned int rotateRight(unsigned int input, unsigned int times)
{
    return (input >> (times % (sizeof(input) * CHAR_BIT))) | (input << ((sizeof(input) * CHAR_BIT) - (times % (sizeof(input) * CHAR_BIT))));
}
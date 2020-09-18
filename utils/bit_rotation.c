#include <limits.h>

#include "bit_rotation.h"

unsigned int rotateLeft(unsigned int input, unsigned int times)
{
    return (input << (times % int_size)) | (input >> (int_size - (times % int_size)));
}

unsigned int rotateRight(unsigned int input, unsigned int times)
{
    return (input >> (times % int_size)) | (input << (int_size - (times % int_size)));
}
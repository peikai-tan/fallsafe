#include <limits.h>

#include "utils/bit_printer.h"
#include "utils/bit_rotation.h"

int main(int agc, char** argv)
{
    unsigned int input = 0xFFFF0000;
    printIntAsBinary(input, int_size);

    for (size_t i = 1; i < int_size; i++)
    {
        printIntAsBinary(rotateLeft(input, i), int_size);
    }

    for (size_t i = 1; i < 32; i++)
    {
        printIntAsBinary(rotateRight(input, i), int_size);
    }

    return 0;
}
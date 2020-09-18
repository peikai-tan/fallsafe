#include <limits.h>

#include "utils/bit_printer.h"
#include "utils/bit_rotation.h"


int main(int agc, char** argv)
{
    unsigned int input = 0xFFFF0000;
    printIntAsBinary(input, 32);

    for (size_t i = 1; i < 32; i++)
    {
        printIntAsBinary(rotateLeft(input, i), 32);
    }

    for (size_t i = 1; i < 32; i++)
    {
        printIntAsBinary(rotateRight(input, i), 32);
    }

    return 0;
}
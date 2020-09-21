#include <stdio.h>

#include "bit_printer.h"

void printIntAsBinary(int input, int length)
{
    do
    {
        // https://stackoverflow.com/questions/21255038/what-does-putchar0-num-do
        putchar('0' + ((input >> (length - 1)) & 1)); // Prints the bit at position from the most to least significant bit
    } while (--length);
    putchar('\n');
}

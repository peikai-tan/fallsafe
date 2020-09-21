#include <stdio.h>

#include "bit_printer.h"

void printIntAsBinary(int input, int length)
{
    do
    {
        putchar(((input >> (length - 1)) & 1) + 48); // Prints the bit at position from the most to least significant bit
    } while (--length);
    putchar('\n');
}
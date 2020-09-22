#include "bit_rotation.h"

#define rotate(input, times, direction_a, direction_b) \
    (input direction_a (times % int_size)) | (input direction_b (int_size - (times % int_size)))

/**
 * rotateLeft() - unsigned int left bit rotation.
 * @input: input integer.
 * @times: number of times to rotate.
 * 
 * Return: rotated output.
*/
unsigned int rotateLeft(unsigned int input, unsigned int times)
{
    return rotate(input, times, <<, >>);
}

/**
 * rotateRight() - unsigned int right bit rotation.
 * @input: input integer.
 * @times: number of times to rotate.
 * 
 * Return: rotated output.
*/
unsigned int rotateRight(unsigned int input, unsigned int times)
{
    return rotate(input, times, >>, <<);
}

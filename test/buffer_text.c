#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void variableStackBuffer(size_t size)
{
    size_t i;
    // From C99 only
    char buffer[size];

    memset(buffer, 0, size * sizeof(char));

    for (i = 0; i < size - 1; i++)
    {
        buffer[i] = '0' + i % 10;
    }

    puts(buffer);
}


int main(void)
{
    int size;
    scanf("%d", &size);
    variableStackBuffer(size);
    return 0;
}
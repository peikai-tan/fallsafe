#include <stdio.h>

#include "common/queue.h"

int main(int agc, char** argv)
{
    Queue *q = queue_new(0);

    queue_destory(q);

    printf("Program finished\n");
    return 0;
}

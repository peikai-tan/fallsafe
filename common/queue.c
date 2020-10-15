#include <stdlib.h>
#include <stdio.h>

#include "queue.h"

/** 
 * Constructs a new queue data structure.
 * @capacity: initial capacity of queue
*/
Queue* queue_new(int capacity)
{
    Queue *output = (Queue *)malloc(sizeof(Queue));

    if (!output)
    {
        fprintf(stderr, "Failure allocating Queue");
    }

    output->length = 0;

    return output;
}

/**
 * Queue destructer
*/
void queue_destory(Queue* queue)
{
    free(queue);
}
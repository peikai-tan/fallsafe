#include <stdlib.h>
#include <stdio.h>

#include "queue.h"

/** 
 * Constructs a new queue data structure.
 * @capacity: initial capacity of queue
*/
Queue queue_new(int capacity, int elementSize)
{
    Queue output = (Queue)malloc(sizeof(struct queue));

    if (!output)
    {
        fprintf(stderr, "Failure allocating Queue");
    }
    output->_list = _arraylist_new(capacity, elementSize);
    output->length = 0;

    return output;
}

/**
 * Queue destructer
*/
void queue_destory(Queue queue)
{
    free(queue);
}

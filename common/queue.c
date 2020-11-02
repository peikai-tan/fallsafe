#include <stdlib.h>
#include <stdio.h>

#include "queue.h"

/** 
 * Constructs a new queue data structure.
 * @capacity: initial capacity of queue
*/
Queue _queue_new(int capacity, int elementSize)
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
 * Queue destructor
*/
void queue_destroy(Queue queue)
{
    arraylist_destroy(queue->_list);
    free(queue);
}

void queue_enqueue(Queue queue, void *item)
{
    arraylist_push(queue->_list, item);
    queue->length = queue->_list->length;
}

void queue_dequeue(Queue queue, void *out)
{
    arraylist_shift(queue->_list, out);
    queue->length = queue->_list->length;
}

void *queue_peek(Queue queue)
{
    return arraylist_elementAt(queue->_list, 0);
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "queue.h"

/** 
 * Constructs a new queue data structure.
 * @capacity: initial capacity of queue
*/
Queue _queue_new(size_t capacity, size_t elementSize)
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

void queue_peekRange(Queue queue, size_t count, void **out)
{
    // Check and set if count is more than the queue length 
    count = count > queue->length ? queue->length : count;

    // There is no offset worry about
    if (queue->_list->_iterator_offset == 0)
    {
        memcpy(*out, queue->_list->_array, count * queue->_list->_element_size);
    }
    // Get the end part of the offset
    size_t endpart = queue->_list->_capacity - queue->_list->_iterator_offset;
    memcpy(*out, voidptr_offset(queue->_list->_array, queue->_list->_iterator_offset * queue->_list->_element_size), endpart * queue->_list->_element_size);
    // There not more elements at the front
    if (endpart >= count)
    {
        return;
    }
    // Get the front part of the offset
    size_t frontpart = count - endpart;
    memcpy(voidptr_offset(*out, endpart * queue->_list->_element_size), queue->_list->_array, frontpart * queue->_list->_element_size);
}

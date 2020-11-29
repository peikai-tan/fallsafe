// This file contains the function definition for the
// queue library.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "queue.h"

/** 
 * _queue_new(): Constructs a new queue data structure.
 * @capacity: initial capacity of queue
 * @elementSize: initial number of elements
 *
 * Allocate memory for the queue first then allocate memory
 * for the list using arraylist constructor.
 *
 * Return: The new queue struct with specified default capacity and element size
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
 * queue_destroy(): Destructor for queue
 * @queue: Queue structure to destroy
 *
 * Free the list before freeing the queue structure.
*/
void queue_destroy(Queue queue)
{
    arraylist_destroy(queue->_list);
    free(queue);
}

/**
 * queue_enqueue(): Enqueue function
 * @queue: Queue structure to enqueue
 * @item: Any item to enqueue
 *
 * Put a new item at the end of the queue
*/
void queue_enqueue(Queue queue, void *item)
{
    arraylist_push(queue->_list, item);
    queue->length = queue->_list->length;
}

/**
 * queue_dequeue(): Dequeue function
 * @queue: Queue structure to dequeue
 * @out: Any item to dequeue
 *
 * Remove the first index object in the queue
*/
void queue_dequeue(Queue queue, void *out)
{
    arraylist_shift(queue->_list, out);
    queue->length = queue->_list->length;
}

/**
 * queue_peek(): Peek function
 * @queue: Queue structure to peek
 *
 * Return the first index object
*/
void *queue_peek(Queue queue)
{
    return arraylist_elementAt(queue->_list, 0);
}

/**
 * queue_peekRange(): Peek a range of items in the queue
 * @queue: Queue structure to peek range
 * @count: How much to peek
 * @out: Pointer to an array of items to return
*/
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

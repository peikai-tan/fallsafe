#if !defined(COMMON_QUEUE)
#define COMMON_QUEUE
// This file contains the function definition for the
// queue library.
#include "./arraylist.h"
/**
 * typedef struct queue - Queue structure
 * @_list: Array list of items
 * @length: Length of the queue
 */
typedef struct queue
{
    ArrayList _list;
    size_t length;
}* Queue;

#define queue_new(type, capacity) \
    _queue_new(capacity, sizeof(type))

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
Queue _queue_new(size_t capacity, size_t elementSize);

/**
 * queue_destroy(): Destructor for queue
 * @queue: Queue structure to destroy
 *
 * Free the list before freeing the queue structure.
*/
void queue_destroy(Queue queue);

/**
 * queue_enqueue(): Enqueue function
 * @queue: Queue structure to enqueue
 * @item: Any item to enqueue
 *
 * Put a new item at the end of the queue
*/
void queue_enqueue(Queue queue, void *item);

/**
 * queue_dequeue(): Dequeue function
 * @queue: Queue structure to enqueue
 * @out: Any item to dequeue
 *
 * Remove the first index object in the queue
*/
void queue_dequeue(Queue queue, void *out);

/**
 * queue_peek(): Peek function
 * @queue: Queue structure to enqueue
 *
 * Return the first index object
*/
void* queue_peek(Queue queue);

/**
 * queue_peekRange(): Peek a range of items in the queue
 * @queue: Queue structure to peek
 * @count: How much to peek
 * @out: Pointer to an array of items to return
*/
void queue_peekRange(Queue queue, size_t count, void **out);

#endif // COMMON_QUEUE

#if !defined(COMMON_QUEUE)
#define COMMON_QUEUE

#include "./arraylist.h"

typedef struct queue
{
    ArrayList _list;
    size_t length;
}* Queue;

#define queue_new(type, capacity) \
    _queue_new(capacity, sizeof(type))

Queue _queue_new(int capacity, int elementSize);
void queue_destroy(Queue queue);

void queue_enqueue(Queue queue, void *item);
void queue_dequeue(Queue queue, void *out);
void* queue_peek(Queue queue);

#endif // COMMON_QUEUE

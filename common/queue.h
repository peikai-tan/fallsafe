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

Queue _queue_new(size_t capacity, size_t elementSize);
void queue_destroy(Queue queue);

void queue_enqueue(Queue queue, void *item);
void queue_dequeue(Queue queue, void *out);
void* queue_peek(Queue queue);
void queue_peekRange(Queue queue, size_t count, void **out);

#endif // COMMON_QUEUE

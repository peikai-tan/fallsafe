#if !defined(COMMON_QUEUE)
#define COMMON_QUEUE

#include "arraylist.h"

typedef struct queue
{
    ArrayList _list;
    int length;
}* Queue;

Queue queue_new(int capacity, int elementSize);
void queue_destory(Queue queue);

void queue_enqueue(Queue queue, void *item);
void queue_dequeue(Queue queue, void *out);
void* queue_peek(Queue queue);

#endif // COMMON_QUEUE

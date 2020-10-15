typedef struct queue {
    int length;
} Queue;

Queue *queue_new(int capacity);
void queue_destory(Queue *queue);

void *queue_enqueue(Queue *queue, void *item);
void *queue_dequeue(Queue *queue);
void *queue_peek(Queue *queue);
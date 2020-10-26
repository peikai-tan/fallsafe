#include <stdio.h>
#include <string.h>

#include "../common/arraylist.h"
#include "../common/queue.h"
#include "../common/memoryUtils.h"

#define TEST_SIZE 0xF

void int_string(char *buffer, void *input)
{
    sprintf(buffer, "%d", *(int *)input);
}

void intArray_rawPrint(ArrayList list)
{
    printf("RAW: [");
    for (size_t i = 0; i < list->_capacity; i++)
    {
        printf(" %d,", ((int *)list->_array)[i]);
    }
    puts("]");
}

int main(void)
{
    int holder = 0;
    char printBuffer[20];
    Queue intQueue = queue_new(int, 1);
    puts("Enqueuing");
    for (int i = 0; i < TEST_SIZE; i++)
    {
        queue_enqueue(intQueue, &i);
    }
    printf("Queue length: %zu ", intQueue->length);
    arraylist_print(intQueue->_list, printBuffer, int_string);
    puts("Dequeuing");
    for (int i = 0; i < TEST_SIZE / 1.5; i++)
    {
        queue_dequeue(intQueue, &holder);
        printf("Queue length: %zu ", intQueue->length);
        arraylist_print(intQueue->_list, printBuffer, int_string);
    }
    puts("enqueue and dequeue");
    for (int i = 0; i < TEST_SIZE / 1.5; i++)
    {
        queue_enqueue(intQueue, &i);
        arraylist_print(intQueue->_list, printBuffer, int_string);
        queue_dequeue(intQueue, &holder);
        printf("Queue length: %zu ", intQueue->length);
        arraylist_print(intQueue->_list, printBuffer, int_string);
    }

    puts("Dequeue and enqueue");
    for (int i = 0; i < TEST_SIZE / 1.5; i++)
    {
        queue_dequeue(intQueue, &holder);
        arraylist_print(intQueue->_list, printBuffer, int_string);
        queue_enqueue(intQueue, &i);
        printf("Queue length: %zu ", intQueue->length);
        arraylist_print(intQueue->_list, printBuffer, int_string);
    }

    puts("More enqueue than dequeue");
    for (int i = 0; i < TEST_SIZE; i++)
    {
        queue_enqueue(intQueue, &i);
        arraylist_print(intQueue->_list, printBuffer, int_string);
        queue_enqueue(intQueue, &i);
        arraylist_print(intQueue->_list, printBuffer, int_string);
        queue_dequeue(intQueue, &holder);
        printf("Queue length: %zu ", intQueue->length);
        arraylist_print(intQueue->_list, printBuffer, int_string);
    }

    puts("Peaking");
    holder = *(int*)queue_peek(intQueue);
    printf("Peaked: %d\n", holder);

    puts("More dequeue than enqueue till none");
    for (int i = 0; intQueue->length != 0; i++)
    {
        queue_enqueue(intQueue, &i);
        arraylist_print(intQueue->_list, printBuffer, int_string);
        queue_dequeue(intQueue, &holder);
        arraylist_print(intQueue->_list, printBuffer, int_string);
        queue_dequeue(intQueue, &holder);
        printf("Queue length: %zu ", intQueue->length);
        arraylist_print(intQueue->_list, printBuffer, int_string);
    }

    queue_destroy(intQueue);
    puts("Program finished");
    return 0;
}
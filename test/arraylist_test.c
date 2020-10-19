#include <stdio.h>

#include "../common/arraylist.h"

#define TEST_SIZE 0x00FFFFFF

int main(void)
{
    ArrayList intList = arraylist_new(int, 0);

    for (size_t i = 0; i < TEST_SIZE; i++)
    {
        arraylist_push(intList, &i);
    }

    for (size_t i = 0; i < TEST_SIZE - 5; i++)
    {
        int popped;
        arraylist_pop(intList, &popped);
        if (popped >= 0 && popped < 10)
        {
            printf("If popped less than 10: %d\n", popped);
        }
    }

    for (size_t i = 0; i < intList->length; i++)
    {
        int *item = (int *)(arraylist_elementAt(intList, i));
        int oldVal = *item;
        *item = *item % 2;
        printf("item[%lu]: %d Mutated: %d\n", (unsigned long) i, oldVal, *(int *)(arraylist_elementAt(intList, i)));
    }

    arraylist_destory(intList);

    printf("Program finished\n");
    return 0;
}

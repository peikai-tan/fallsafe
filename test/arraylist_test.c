#include <stdio.h>

#include "../common/arraylist.h"

#define TEST_SIZE 1000000

int main(int agc, char **argv)
{
    ArrayList *intList = arraylist_new(0, int);

    for (int i = 0; i < TEST_SIZE; i++)
    {
        arraylist_push(intList, &i);
    }

    for (int i = 0; i < TEST_SIZE - 5; i++)
    {
        int popped;
        arraylist_pop(intList, &popped);
        if (popped < 10)
        {
            printf("popped at %d\n", popped);
        }
    }

    for (int i = 0; i < intList->length; i++)
    {
        printf("item[%d]: %d\n", i, *(int*)(arraylist_elementAt(intList, i)));
    }

    arraylist_destory(intList);

    printf("Program finished\n");
    return 0;
}

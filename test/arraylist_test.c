#include <stdio.h>
#include <string.h>

#include "../common/arraylist.h"
#include "../common/memoryUtils.h"

#define TEST_SIZE 0xFF

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

void intTest()
{
    ArrayList intList;
    puts("Test int push pop ===================================");
    intList = arraylist_new(int, 1);

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
        printf("item[%lu]: %d Mutated: %d\n", (unsigned long)i, oldVal, *(int *)(arraylist_elementAt(intList, i)));
    }

    for (size_t i = 10; i < 20; i++)
    {
        arraylist_push(intList, &i);
    }

    char printbuffer[20];
    arraylist_print(intList, printbuffer, int_string);

    arraylist_destroy(intList);

    puts("Test shift unshift ==================================");

    intList = arraylist_new(int, 16);
    int holder = 0;

    for (int i = 1; i < 10; i++)
    {
        arraylist_unshift(intList, &i);
        arraylist_print(intList, printbuffer, int_string);
    }

    for (int i = 0; i < 3; i++)
    {
        arraylist_push(intList, &i);
        arraylist_print(intList, printbuffer, int_string);
    }

    for (int i = 10; i < 16; i++)
    {
        arraylist_unshift(intList, &i);
        arraylist_print(intList, printbuffer, int_string);
    }

    arraylist_pop(intList, &holder);

    holder = -1;
    arraylist_unshift(intList, &holder);
    arraylist_print(intList, printbuffer, int_string);

    for (size_t i = 0; i < 12; i++)
    {
        arraylist_shift(intList, &holder);
        arraylist_print(intList, printbuffer, int_string);
    }
    intArray_rawPrint(intList);

    holder = -1;
    arraylist_unshift(intList, &holder);
    arraylist_print(intList, printbuffer, int_string);

    intArray_rawPrint(intList);

    arraylist_shift(intList, &holder);
    intArray_rawPrint(intList);
    size_t length = intList->length;
    for (size_t i = 0; i < length; i++)
    {
        arraylist_pop(intList, &holder);
    }
    intArray_rawPrint(intList);
    arraylist_print(intList, printbuffer, int_string);

    arraylist_destroy(intList);
}

void char_string(char *buffer, void *input)
{
    sprintf(buffer, "%s", *(char **)input);
}

void stringTest()
{
    ArrayList stringList = arraylist_new(char **, TEST_SIZE * 2);
    char test[10] = "test";
    char printBuffer[10];

    for (int i = 0; i < TEST_SIZE; i++)
    {
        char *input = (char *)malloc(20);
        sprintf(input, "%s %d", test, i);
        arraylist_push(stringList, &input);
    }

    //arraylist_print(stringList, printBuffer, char_string);

    for (size_t i = 0; i < TEST_SIZE; i++)
    {
        char *elem = *(char **)arraylist_elementAt(stringList, i);
        free(elem);
    }

    arraylist_destroy(stringList);
}

int main(void)
{
    intTest();
    //stringTest();
    printf("Program finished\n");
    return 0;
}

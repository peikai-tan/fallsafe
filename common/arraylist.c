#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "./arraylist.h"
#include "./memory.h"

// #define DEBUG

#define list_position(list, index) \
    (((list->_iterator_offset + index) % list->_capacity) * list->_element_size)

static bool need_expand(ArrayList list)
{
    return list->length == list->_capacity;
}

static bool need_shrink(ArrayList list)
{
    return list->_capacity > 1 ? (double)list->length / (double)list->_capacity < 0.25 : false;
}

/**
 * compact() - Decrease size of ArrayList
 * @list - Target ArrayList of operation
 * @target_size - Specified size of shrinking the ArrayList
 *
 * Example array of 16 capacity with 9 element and non zero offset to target size 10
 * Index        : 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * Content      : 4 5 6 7 8 - - - - - - - 0 1 2 3
 * First pass   : 4 5 6 7 8 - 0 1 2 3 - - - - - -
 * realloc      : 4 5 6 7 8 - 0 1 2 3
 *
 * Return - A new ArrayList in the specified target size
 */
static ArrayList compact(ArrayList list, size_t target_size)
{
    size_t distance = list->_capacity - target_size;
    size_t newOffset = distance > list->_iterator_offset ? 0 : list->_iterator_offset - distance;
    size_t offsetPartLength = list->_capacity - list->_iterator_offset;

    void *source = voidptr_offset(list->_array, list->_iterator_offset * list->_element_size);
    void *destination = voidptr_offset(list->_array, newOffset * list->_element_size);

    void *moved = memmove(destination, source, offsetPartLength * list->_element_size);

    if (!moved)
    {
        fprintf(stderr, "Failure memory move compact arraylist resize\n");
        return NULL;
    }

    list->_iterator_offset = newOffset;
    list->_array = realloc(list->_array, target_size * list->_element_size);

    if (!list->_array)
    {
        fprintf(stderr, "Failure compacting arraylist resize\n");
        return NULL;
    }

    return list;
}

/**
 * widen() - Expand ArrayList
 * @list - Target ArrayList of operation
 * @target_size - Specified size to expand to
 *
 * Example array of 16 capacity with 9 element and non zero offset to target size 16
 * Index        : 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * Content      : 4 5 6 7 8 - 0 1 2 3
 * realloc      : 4 5 6 7 8 - 0 1 2 3 - - - - - -
 * First pass   : 4 5 6 7 8 - - - - - - - 0 1 2 3
 *
 * Return - A new ArrayList in the specified target size
 */
static ArrayList widen(ArrayList list, size_t target_size)
{
    size_t expansionSize = target_size - list->_capacity;
    size_t offsetPartLength = list->_capacity - list->_iterator_offset;
    size_t newOffset = list->_iterator_offset + expansionSize;

    list->_array = realloc(list->_array, target_size * list->_element_size);

    if (!list->_array)
    {
        fprintf(stderr, "Failure spacing out arraylist resize\n");
        return NULL;
    }

    void *source = voidptr_offset(list->_array, list->_iterator_offset * list->_element_size);
    void *destination = voidptr_offset(list->_array, newOffset * list->_element_size);
    memmove(destination, source, offsetPartLength * list->_element_size);

    list->_iterator_offset = newOffset;

    return list;
}

/**
 * resize() - Adjust size of ArrayList
 * @list - Target ArrayList of operation
 * @size - Specified size to expand to
 *
 *
 *
 * Return - A new ArrayList in the specified target size
 */
static ArrayList resize(ArrayList list, size_t size)
{
    if (list->_iterator_offset == 0)
    {
        list->_array = realloc(list->_array, size * list->_element_size);

        if (!list->_array)
        {
            fprintf(stderr, "Failure reallocating arraylist resize\n");
            return NULL;
        }
    }
    else
    {
        if (size > list->_capacity)
        {
            widen(list, size);
        }
        else
        {
            compact(list, size);
        }
    }
    list->_capacity = size;

#if defined(DEBUG)
    printf("resized list to %zu (%zu bytes)\n", size, size * CHAR_BIT);
#endif // DEBUG
    return list;
}

/**
 * _arraylist_new() - Create new ArrayList
 * @capacity - Length of ArrayList
 * @element_size - Size of each element of ArrayList
 *
 * Return - New ArrayList
 */
ArrayList _arraylist_new(size_t capacity, size_t element_size)
{
    ArrayList list = (ArrayList)malloc(sizeof(struct arraylist));
    capacity = capacity < 1 ? 1 : capacity;

    if (!list)
    {
        fprintf(stderr, "Failure allocating arraylist container\n");
        return NULL;
    }

    list->length = 0;
    list->_element_size = element_size;
    list->_iterator_offset = 0;
    list->_capacity = capacity;
    list->_array = calloc(capacity, element_size);

    if (!list->_array)
    {
        fprintf(stderr, "Failure allocating arraylist items\n");
    }

    return list;
}

/**
 * arraylist_destroy() - Destroy ArrayList
 * @list - Target ArrayList of operation
 *
 * Remove ArrayList through freeing pointer
 */
void arraylist_destroy(ArrayList list)
{
    free(list->_array);
    free(list);
}

/**
 * arraylist_elementAt() - Get element at position in ArrayList
 * @list - Target ArrayList of operation
 * @index - Index of element in ArrayList
 *
 */
void *arraylist_elementAt(ArrayList list, size_t index)
{
    if (index >= list->length)
    {
        return NULL;
    }

    size_t position = list_position(list, index);

    return (void *)(((char *)list->_array) + position);
}

/**
 * arraylist_push() - Adds an element to it's end
 * @list - Target ArrayList of operation
 * @item - Data to be added
 *
 */
void arraylist_push(ArrayList list, void *item)
{
    if (need_expand(list))
    {
        resize(list, list->_capacity * 2);
    }

    memcpy(voidptr_offset(list->_array, list_position(list, list->length++)), item, list->_element_size);
}
/**
 * arraylist_pop() - Removes an element from it's end 
 * @list - Target ArrayList of operation
 * @out - Data to be removed
 *
*/
void arraylist_pop(ArrayList list, void *out)
{
    size_t position = list_position(list, --list->length);
    void *output = voidptr_offset(list->_array, position);
    memcpy(out, output, list->_element_size);
    memset(voidptr_offset(list->_array, position), 0, list->_element_size);

    if (need_shrink(list))
    {
        resize(list, list->_capacity / 2);
    }
}

/**
 * arraylist_shift() - Remove index 0 element from the ArrayList
 * @list - Target ArrayList of operation
 * @out - Item to be
 */
void arraylist_shift(ArrayList list, void *out)
{
    size_t position = list_position(list, 0);
    void *output = voidptr_offset(list->_array, position);
    --list->length;
    list->_iterator_offset = list->_iterator_offset == list->_capacity - 1 ? 0 : list->_iterator_offset + 1;
    memcpy(out, output, list->_element_size);
    memset(voidptr_offset(list->_array, position), 0, list->_element_size);
    if (need_shrink(list))
    {
        resize(list, list->_capacity / 2);
    }
}

/**
 * arraylist_unshift() - Add element to index 0 from the ArrayList
 * @list - Target ArrayList of operation
 * @item - Item to be added
 */
void arraylist_unshift(ArrayList list, void *item)
{
    if (need_expand(list))
    {
        resize(list, list->_capacity * 2);
    }
    list->_iterator_offset = list->_iterator_offset == 0 ? list->_capacity - 1 : list->_iterator_offset - 1;
    memcpy(voidptr_offset(list->_array, list->_iterator_offset * list->_element_size), item, list->_element_size);
    ++list->length;
}

/**
 * arraylist_print() - Print elements in the Arraylist
 * @list - Target ArrayList of operation
 * @elementBuffer -
 * @printer - 
 */
void arraylist_print(ArrayList list, char *elementBuffer, void (*printer)(char *, void *))
{
    printf("[");
    for (size_t i = 0; i < list->length; i++)
    {
        void *item = arraylist_elementAt(list, i);
        memset(elementBuffer, 0, strlen(elementBuffer));
        printer(elementBuffer, item);

        if (i == list->length - 1)
        {
            printf(" %s ", elementBuffer);
        }
        else
        {
            printf(" %s,", elementBuffer);
        }
    }
    puts("]");
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "arraylist.h"

#define voidptr_offset(voidptr, offset) \
    ((void *)(((char *)(voidptr)) + (offset)))

#define list_position(list, index) \
    (((list->_iterator_offset + index) % list->_capacity) * list->_element_size)

static bool need_expand(ArrayList list)
{
    return (double)list->length / (double)list->_capacity > 0.75;
}

static bool need_shrink(ArrayList list)
{
    return list->_capacity > 1 ? (double)list->length / (double)list->_capacity < 0.25 : false;
}

static ArrayList resize(ArrayList list, size_t size)
{
    list->_array = realloc(list->_array, size * list->_element_size);

    if (!list->_array)
    {
        fprintf(stderr, "Failure allocating arraylist resize");
        return NULL;
    }

    list->_capacity = size;
    printf("resized list to %zu (%zu bytes)\n", size, size * CHAR_BIT);
    return list;
}

ArrayList _arraylist_new(size_t capacity, size_t element_size)
{
    ArrayList list = (ArrayList)malloc(sizeof(struct arraylist));
    capacity = capacity < 1 ? 1 : capacity;

    if (!list)
    {
        fprintf(stderr, "Failure allocating arraylist container");
        return NULL;
    }

    list->length = 0;
    list->_element_size = element_size;
    list->_iterator_offset = 0;
    list->_capacity = capacity;
    list->_array = calloc(capacity, element_size);

    if (!list->_array)
    {
        fprintf(stderr, "Failure allocating arraylist items");
    }

    return list;
}

void arraylist_destory(ArrayList list)
{
    free(list->_array);
    free(list);
}

void *arraylist_elementAt(ArrayList list, size_t index)
{
    if (index >= list->length)
    {
        return NULL;
    }

    return ((char *)list->_array) + list_position(list, index);
}

/**
 * Adds an element to it's end
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
 * Removes an element from it's end 
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

void arraylist_shift(ArrayList list, void *out);
void arraylist_unshift(ArrayList list, void *item);

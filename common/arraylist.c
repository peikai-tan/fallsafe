#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "arraylist.h"

static bool need_expand(ArrayList *list)
{
    return (double)list->length / (double)list->_capacity > 0.75;
}

static bool need_shrink(ArrayList *list)
{
    return list->_capacity > 1 ? (double)list->length / (double)list->_capacity < 0.25 : false;
}

static ArrayList *resize(ArrayList *list, int size)
{
    list->_array = realloc(list->_array, size * list->_element_size);
    list->_capacity = size;
    printf("resized list to %d\n", size);
}

ArrayList *_arraylist_new(int capacity, int element_size)
{
    ArrayList *list = (ArrayList *)malloc(sizeof(ArrayList));
    capacity = capacity < 1 ? 1 : capacity;

    if (!list)
    {
        fprintf(stderr, "Failure allocating arraylist container");
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

void arraylist_destory(ArrayList *list)
{
    free(list->_array);
    free(list);
}

void *arraylist_elementAt(ArrayList *list, int index)
{
    if (index >= list->length)
    {
        return NULL;
    }

    return list->_array + ((list->_iterator_offset + index) % list->_capacity) * list->_element_size;
}

/**
 * Adds an element to it's end
*/
void arraylist_push(ArrayList *list, void *item)
{
    if (need_expand(list))
    {
        resize(list, list->_capacity * 2);
    }

    memcpy(list->_array + ((list->_iterator_offset + list->length++) % list->_capacity) * list->_element_size, item, list->_element_size);
}
/**
 * Removes an element from it's end 
*/
void arraylist_pop(ArrayList *list, void *out)
{
    int position = ((list->_iterator_offset + --list->length) % list->_capacity) * list->_element_size;
    void *output = list->_array + position;
    memcpy(out, output, list->_element_size);
    memset(list->_array + position, 0, list->_element_size);

    if (need_shrink(list))
    {
        resize(list, list->_capacity / 2);
    }
}

void arraylist_shift(ArrayList *list, void *out);
void arraylist_unshift(ArrayList *list, void *item);

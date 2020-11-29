#if !defined(COMMON_ARRAYLIST)
#define COMMON_ARRAYLIST

#include <stdlib.h>

typedef struct arraylist
{
    void *_array; /* The actual array pointer*/
    size_t _element_size;
    size_t _iterator_offset; /* The actual position where the index '0' is. Usually is 0 but varies if shift and unshift used to operate the list. */
    size_t _capacity;
    size_t length;
}* ArrayList;

/**
 * Generic arraylist constructor wrapper to conveniently take in the actual type rather than sizeof the type
 * Example if in C#: List<int> list = new List<int>(0);
 * Equivalent here: ArrayList list = arraylist_new(int, 0);
*/
#define arraylist_new(type, capacity) \
    _arraylist_new(capacity, sizeof(type))

ArrayList _arraylist_new(size_t capacity, size_t element_size);
/**
 * arraylist_destroy() - Destroy ArrayList
 * @list - Target ArrayList of operation
 *
 * Remove ArrayList through freeing pointer
 */
void arraylist_destroy(ArrayList list);

/**
 * arraylist_elementAt() - Get element at position in ArrayList
 * @list - Target ArrayList of operation
 * @index - Index of element in ArrayList
 *
 */
void* arraylist_elementAt(ArrayList list, size_t index);

/** AKA append */
/**
 * arraylist_push() - Adds an element to it's end
 * @list - Target ArrayList of operation
 * @item - Data to be added
 *
 */
void arraylist_push(ArrayList list, void *item);
/** Remove last element */
/**
 * arraylist_pop() - Removes an element from it's end 
 * @list - Target ArrayList of operation
 * @out - Data to be removed
 *
*/
void arraylist_pop(ArrayList list, void* out);
/** AKA prepend */
/**
 * arraylist_unshift() - Add element to index 0 from the ArrayList
 * @list - Target ArrayList of operation
 * @item - Item to be added
 */
void arraylist_unshift(ArrayList list, void *item);
/** Remove first element */
/**
 * arraylist_shift() - Remove index 0 element from the ArrayList
 * @list - Target ArrayList of operation
 * @out - Item to be
 */
void arraylist_shift(ArrayList list, void *out);

/**
 * arraylist_print() - Print elements in the Arraylist
 * @list - Target ArrayList of operation
 * @elementBuffer -
 * @printer - 
 */
void arraylist_print(ArrayList list, char *elementBuffer, void (*printer)(char *, void *));

#endif // COMMON_ARRAYLIST

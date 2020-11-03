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
void arraylist_destroy(ArrayList list);

void* arraylist_elementAt(ArrayList list, size_t index);

/** AKA append */
void arraylist_push(ArrayList list, void *item);
/** Remove last element */
void arraylist_pop(ArrayList list, void* out);
/** AKA prepend */
void arraylist_unshift(ArrayList list, void *item);
/** Remove first element */
void arraylist_shift(ArrayList list, void *out);

void arraylist_print(ArrayList list, char *elementBuffer, void (*printer)(char *, void *));

#endif // COMMON_ARRAYLIST

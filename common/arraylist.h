#if !defined(COMMON_ARRAYLIST)
#define COMMON_ARRAYLIST

typedef struct arraylist
{
    void *_array; /* The actual array pointer*/
    int _element_size;
    int _iterator_offset; /* The actual position where the index '0' is. Usually is 0 but varies if shift and unshift used to operate the list. */
    int _capacity;
    int length;
} ArrayList;

#define arraylist_new(capacity, type) \
    _arraylist_new(capacity, sizeof(type))

ArrayList *_arraylist_new(int capacity, int element_size);
void arraylist_destory(ArrayList *list);

void* arraylist_elementAt(ArrayList *list, int index);

void arraylist_push(ArrayList *list, void *item);
void arraylist_pop(ArrayList *list, void* out);
void arraylist_unshift(ArrayList *list, void *item);
void arraylist_shift(ArrayList *list, void *out);

#endif // COMMON_ARRAYLIST

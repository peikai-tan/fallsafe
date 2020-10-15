#if !defined(COMMON_ARRAYLIST)
#define COMMON_ARRAYLIST

typedef struct arraylist
{
    void *_array;
    int length;
} ArrayList;

ArrayList *arraylist_new(int capacity);
void arraylist_destory(ArrayList *list);

void arraylist_push(ArrayList *list, void *item);
void *arraylist_pop();
void *arraylist_shift();
void arraylist_unshift(ArrayList *list, void *item);
void arraylist_elememtAt(ArrayList *list, int index);

#endif // COMMON_ARRAYLIST

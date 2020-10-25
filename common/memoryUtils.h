#if !defined(COMMON_MEMORYUTILS)
#define COMMON_MEMORYUTILS

#include <stdlib.h>

#define voidptr_offset(voidptr, offset) \
    ((void *)(((char *)(voidptr)) + (offset)))

void swap(void *const a, void *const b, const size_t size);

#endif // COMMON_MEMORYUTILS

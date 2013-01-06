#include <stddef.h>

typedef struct StringBuffer {
    char *buffer;
    int size;
    int capacity;
} StringBuffer;

StringBuffer
*StringBuffer_construct();

StringBuffer
*StringBuffer_construct_n(int capacity);

StringBuffer
*StringBuffer_concat(StringBuffer *ptr, char *string);

StringBuffer
*StringBuffer_concat_n(StringBuffer *ptr, char *string, size_t len);

int
StringBuffer_resize(StringBuffer *ptr, int newCapacity);

void
StringBuffer_free(StringBuffer *ptr);

void
StringBuffer_clear(StringBuffer *ptr);

#include "StringBuffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define STANDARD_SIZE 16

StringBuffer
*StringBuffer_construct() {
    return StringBuffer_construct_n(STANDARD_SIZE);
}

StringBuffer
*StringBuffer_construct_n(int capacity) {
    
    StringBuffer *ptr = malloc(sizeof(StringBuffer));
    if (ptr == NULL) {
        perror("Insufficient memory!");
        return NULL;
    }
    char *buffer = malloc((sizeof(char) * capacity) + (1 * sizeof(char)));
    if (buffer == NULL) {
        perror("Insufficient memory!");
        return NULL;
    }
    buffer[capacity] = '\0';
    ptr->buffer = buffer;
    ptr->size = 0;
    ptr->capacity = capacity;
    
    return ptr;
}

StringBuffer
*StringBuffer_concat(StringBuffer *ptr, char *string) {
    return StringBuffer_concat_n(ptr, string, strlen(string));
}

StringBuffer
*StringBuffer_concat_n(StringBuffer *ptr, char *string, size_t len) {
    // Need to resize the buffer
    // Always the double len
    if (ptr->size + len > ptr->capacity) {
        // Search for the new capacity...maybe we can solve this with mathematica...
        int newCapacity = ptr->capacity;
        int t = ptr->size + len;
        do {
            newCapacity = newCapacity << 1;
        } while(newCapacity < t);
        // Increase the buffer
        if (StringBuffer_resize(ptr, newCapacity) == EXIT_FAILURE) {
            return NULL;
        }
    }
    // Concat the string
    char *start = ptr->buffer + ptr->size;
    memcpy(start, string, len);
    ptr->size = ptr->size + len;
    ptr->buffer[ptr->size] = '\0';
    return ptr;
}

int
StringBuffer_resize(StringBuffer *ptr, int newCapacity) {
    // Allocate new memory
    char *buffer = realloc(ptr->buffer, (sizeof(char) * newCapacity) + (1 * sizeof(char)));
    buffer[newCapacity] = '\0';
    if (buffer == NULL) {
        perror("Insufficient memory!");
        return EXIT_FAILURE;        
    }
    ptr->buffer = buffer;
    ptr->capacity = newCapacity;
    return EXIT_SUCCESS;
}

void
StringBuffer_free(StringBuffer *ptr) {
    free(ptr->buffer);
    free(ptr);
}

void
StringBuffer_clear(StringBuffer *ptr) {
    if (ptr->size > 0)
        ptr->buffer[0] = '\0';
    ptr->size = 0;
}

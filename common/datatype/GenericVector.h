#include <stdlib.h> 

#define DefVector(type) \
typedef struct T_##type##Vector { \
    type *elements; \
    int size, capacity; \
} type##Vector; \
\
static type##Vector* \
type##Vector_construct(int capacity){ \
    type##Vector *s = malloc(sizeof(type##Vector)); \
    if (s == NULL) \
        return NULL; \
    s->elements = malloc(sizeof(type) * capacity); \
    if (s->elements == NULL) { \
        free(s); \
        return NULL; } \
    s->size=0; \
    s->capacity = capacity; \
    return s; \
} \
\
\
static int \
type##Vector_resize(type##Vector *vector, int newSize) { \
    if (vector == NULL) { \
        perror("vector is null!"); \
        return EXIT_FAILURE; \
    } \
    /* Resize the vector with double size */ \
    type *ptr = realloc(vector->elements, sizeof(type) * newSize); \
    if (ptr == NULL) { \
        perror("Insufficent memory!"); \
        return EXIT_FAILURE; \
    } \
    vector->elements = ptr; \
    vector->capacity = newSize; \
    return EXIT_SUCCESS; \
} \
\
static int \
type##Vector_add(type##Vector *vector, type e) { \
    if (vector == NULL) { \
        perror("vector is null!"); \
        return EXIT_FAILURE; \
    } \
    /* Vector is full */ \
    if (vector->size == vector->capacity) { \
        if (type##Vector_resize(vector, vector->capacity * 2) == EXIT_FAILURE) { \
            return EXIT_FAILURE; \
        } \
    } \
    \
    /* Add element to vector */ \
    vector->elements[vector->size] = e; \
    vector->size = vector->size + 1; \
    return EXIT_SUCCESS; \
} \
\
static type* \
type##Vector_get(type##Vector *vector, int index) { \
    if (vector == NULL) { \
        perror("vector is null!"); \
        return NULL; \
    } \
    if (index < 0 || index >= vector->size) { \
        fprintf(stderr , "Index %d outside of vector (size = %d)\n", index, vector->size); \
        return NULL; \
    } \
    return &(vector->elements[index]); \
} \
\
static type* \
type##Vector_removeAt(type##Vector *vector, int index, type *holder) { \
    if (vector == NULL) { \
        perror("vector is null!"); \
        return NULL; \
    } \
    if (index < 0 || index >= vector->size) { \
        fprintf(stderr , "Index %d outside of vector (size = %d)\n", index, vector->size); \
        return NULL; \
    } \
    /* Only store removed element when interessted in */ \
    if (holder != NULL) \
        *holder = vector->elements[index]; \
    /* Remove element and shift all other elements on the left */ \
    int i = index; \
    for (; i < vector->size - 1 ; ++i) { \
        vector->elements[i] = vector->elements[i + 1]; \
    } \
    vector->size = vector->size - 1; \
    /* Vector is only a quarter full -> resize it to half size */ \
    if ((vector->capacity / 4) == vector->size) { \
        if (type##Vector_resize(vector, vector->capacity / 2) == EXIT_FAILURE) { \
            return NULL; \
        } \
    } \
    \
    return holder; \
} \
\
static int \
type##Vector_remove(type##Vector *vector, type *e, int (*equals)(type *e1, type *e2)) { \
    if (vector == NULL) { \
        perror("vector is null!"); \
        return EXIT_FAILURE; \
    } \
    int i; \
    for (i = 0 ; i < vector->size; ++i) { \
        if (((*equals)(e, &(vector->elements[i]))) == 0) { \
            type##Vector_removeAt(vector, i, NULL); \
            return EXIT_SUCCESS; \
        } \
    } \
    return EXIT_FAILURE;\
} \
\
static int \
type##Vector_contains(type##Vector *vector, type *e, int (*equals)(type *e1, type *e2)) { \
    if (vector == NULL) { \
        perror("vector is null!"); \
        return EXIT_FAILURE; \
    } \
    int i; \
    for (i = 0 ; i < vector->size; ++i) { \
        if (((*equals)(e, &(vector->elements[i]))) == 0) { \
            return EXIT_SUCCESS; \
        } \
    } \
    return EXIT_FAILURE;\
}

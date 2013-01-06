#include <stdlib.h> 

#define DefVector(type, name) \
typedef struct T_##name##Vector { \
    type *elements; \
    int size, capacity; \
} name##Vector; \
\
static name##Vector* \
name##Vector_construct(int capacity){ \
    name##Vector *s = malloc(sizeof(name##Vector)); \
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
name##Vector_resize(name##Vector *vector, int newSize) { \
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
name##Vector_add(name##Vector *vector, type e) { \
    if (vector == NULL) { \
        perror("vector is null!"); \
        return EXIT_FAILURE; \
    } \
    /* Vector is full */ \
    if (vector->size == vector->capacity) { \
        if (name##Vector_resize(vector, vector->capacity * 2) == EXIT_FAILURE) { \
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
name##Vector_get(name##Vector *vector, int index) { \
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
name##Vector_removeAt(name##Vector *vector, int index, type *holder) { \
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
        if (name##Vector_resize(vector, vector->capacity / 2) == EXIT_FAILURE) { \
            return NULL; \
        } \
    } \
    \
    return holder; \
} \
\
static int \
name##Vector_remove(name##Vector *vector, type *e, int (*equals)(type *e1, type *e2)) { \
    if (vector == NULL) { \
        perror("vector is null!"); \
        return EXIT_FAILURE; \
    } \
    int i; \
    for (i = 0 ; i < vector->size; ++i) { \
        if (((*equals)(e, &(vector->elements[i]))) == 0) { \
            name##Vector_removeAt(vector, i, NULL); \
            return EXIT_SUCCESS; \
        } \
    } \
    return EXIT_FAILURE;\
} \
\
static int \
name##Vector_contains(name##Vector *vector, type *e, int (*equals)(type *e1, type *e2)) { \
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

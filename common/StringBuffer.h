/*
 * Copyright (C) 2012 Kilian Gärtner
 * 
 * This file is part of Gnuddels.
 * 
 * Gnuddels is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * 
 * Gnuddels is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Gnuddels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>
#ifndef STRINGBUFFER_h
#define STRINGBUFFER_h

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

#endif

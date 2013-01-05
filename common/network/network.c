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
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <netinet/in.h>

#include "network.h"

#define OUT_BUFFER_SIZE 4096

int transferFile(int source, int destination, char *buffer) {
    
    memset(buffer, 0, OUT_BUFFER_SIZE);
    int bytes_read;
    while((bytes_read = read(source, buffer, OUT_BUFFER_SIZE)) > 0) {
        if (sendAll(destination, buffer, bytes_read) == -1)
            return -1;
    }

    return EXIT_SUCCESS;
}

int sendAll(int dest, char *data, int dataLength) {
    int sent = send(dest, data, dataLength,MSG_NOSIGNAL );
    if (sent == -1)
        return EXIT_FAILURE;
    if (sent != dataLength) {
        char *p = data;
        do {
            dataLength -= sent;
            p += sent;
            sent = send(dest, p, dataLength,MSG_NOSIGNAL);
            if (sent == -1)
                return EXIT_FAILURE;
        } while (dataLength > 0);
    }

    return EXIT_SUCCESS;
}

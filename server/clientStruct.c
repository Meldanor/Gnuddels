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

#include "clientStruct.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// function to create a struct for multiplexing I/O
int createClientStruct(struct client *client, int clientSocket, struct sockaddr_in *conInfo) {
    // assign values
    client->isConnected = true;
    client->socket = clientSocket;
    client->conInfo = conInfo;

    // create buffer and assign values
    char *ptr;

    // in buffer
    ptr = calloc(sizeof(char), IN_BUFFER_SIZE + 1);
    //ptr = malloc(sizeof (char) * (IN_BUFFER_SIZE + 1 ));
    if (ptr == NULL) {
        free(client);
        perror("Can't allocate memory for client in buffer");
        return EXIT_FAILURE;
    }
    //memset(bufferPointer, 0, sizeof(bufferPointer));
    client->inBuffer = ptr;

    // out buffer
    ptr = calloc(sizeof(char), OUT_BUFFER_SIZE+1);
    //ptr = malloc(sizeof (char) * (OUT_BUFFER_SIZE + 1 ));
    if (ptr == NULL) {
        free(client);
        free(client);
        perror("Can't allocate memory for client in buffer");
        return EXIT_FAILURE;
    }
    //memset(bufferPointer, 0, sizeof(ptr));
    client->outBuffer = ptr;

    return EXIT_SUCCESS;
}

/* Function to free memory and clear up the struct */
void freeClient(struct client *client) {
    // Nothing to do
    if (client == NULL) {
        return;
    }

    close(client->socket);
    // free inBuffer
    if (client->inBuffer != NULL) {
        free(client->inBuffer);
    }
    // free outBuffer
    if (client->outBuffer != NULL) {
        free(client->outBuffer);
    }
    // free information about the client connection
    if (client->conInfo != NULL) {
        free(client->conInfo);
    }
    // free the struct itself
    free(client);
}

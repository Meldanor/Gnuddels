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

#include "clientData.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* function to encapsulate the necessary multithreaded information */
int getClientData(struct clientData *clientData, int clientSocket, struct sockaddr_in *connectionInformation) {
    // assign values
    clientData->isConnected = true;
    clientData->clientSocket = clientSocket;
    clientData->connectionInformation = connectionInformation;

    // create buffer and assign values
    char *bufferPointer;

    // in buffer
    bufferPointer = malloc(sizeof (char) * (IN_BUFFER_SIZE + 1 ));
    if (bufferPointer == NULL) {
        free(clientData);
        perror("Can't allocate memory for the terminalClient inBuffer!");
        return EXIT_FAILURE;
    }
    memset(bufferPointer, 0, sizeof(bufferPointer));
    clientData->inBuffer = bufferPointer;

    // out buffer
    bufferPointer = malloc(sizeof (char) * (OUT_BUFFER_SIZE + 1 ));
    if (bufferPointer == NULL) {
        free(clientData->inBuffer);
        free(clientData);
        perror("Can't allocate memory for the terminalClient outBuffer!");
        return EXIT_FAILURE;
    }
    memset(bufferPointer, 0, sizeof(bufferPointer));
    clientData->outBuffer = bufferPointer;

    return EXIT_SUCCESS;
}

/* Function to free memory and clear up the struct */
void clearClient(struct clientData *clientData) {
    // Nothing to do
    if (clientData == NULL) {
        return;
    }
    // free inBuffer
    if (clientData->inBuffer != NULL) {
        free(clientData->inBuffer);
    }
    // free outBuffer
    if (clientData->outBuffer != NULL) {
        free(clientData->outBuffer);
    }
    // free thread memory
    if (clientData->thread != NULL) {
        free(clientData->thread);
    }
    // free information about the client connection
    if (clientData->connectionInformation != NULL) {
        free(clientData->connectionInformation);
    }
    // free the struct itself
    free(clientData);
}

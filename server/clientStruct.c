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

Client
*Client_construct(int clientSocket, char *name) {
    Client *client = malloc(sizeof(client));
    if (client == NULL) {
        perror("Insufficent memory!");
        return NULL;
    }
    client->socket = clientSocket;
    client->name = name;
    StringBuffer *buffer = StringBuffer_construct_n(4096);
    if (buffer == NULL) {
        return NULL;
    }
    client->buffer = buffer;
    
    return client;
}

void
Client_free(Client *client) {
    free(client->name);
    StringBuffer_free(client->buffer);
    free(client);
}

int
equals_Client_Socket(Client *c1, Client *c2) {
    return (c1->socket == c2->socket ? 0 : -1);
}

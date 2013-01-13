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

#ifndef CLIENTSTRUCT_H
#define CLIENTSTRUCT_H

#include <stdbool.h>
#include <stdlib.h>
#include "../common/StringBuffer.h"

typedef struct Client {
    int socket;
    StringBuffer *buffer;
    char *name;
} Client;

Client
*Client_construct(int clientSocket, char *name);

void
Client_setName(Client *client, char *name);

void
Client_free(Client *client);

int
equals_Client_Socket(Client *c1, Client *c2);

int
equals_Client_Name(Client *c1, Client *c2);

#endif

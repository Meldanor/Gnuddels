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

#include <pthread.h>
#include <stdbool.h>

#define IN_BUFFER_SIZE 4096
#define OUT_BUFFER_SIZE 4096

struct clientData {
    int clientSocket;
    bool isConnected;
    struct sockaddr_in *connectionInformation;
    pthread_t *thread;
    char *inBuffer;
    char *outBuffer;
    int position;
};

int getClientData(struct clientData *clientData, int clientSocket, struct sockaddr_in *connectionInformation);

void clearClient(struct clientData *clientData);

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

#include <netinet/in.h>

int parseArguments(int argc, char **args, char **port);

int initConnection(char *port);

void serverLoop(void);

void stopServer(int signal);

int addClient(int clientSocket, struct sockaddr_in *conInfo);

void removeClient(struct client *client);

void handleClient(struct client *client);

int readFromClient(struct client *client, int *recBytes);

int increasePollArray(int newFileDescriptor);

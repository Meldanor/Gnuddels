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

// Methods only called when server is starting / stopping

int init(int argc, char **args);

int parseArguments(int argc, char **args, char **port);

int initConnection(char *port);

int initPoll();

void stopServer(void);

// Methods called when the server is running

void serverLoop(void);

int accept_newClient();

int remove_client(int socket);

int handle_client(int socket);

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

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "server/server.h"

// Just the main for the compiling
// Everything else happens in the "server/server.c" file
int main(int argc, char **args) {

    char *port;
    if (parseArguments(argc, args, &port) == EXIT_FAILURE)
        return EXIT_FAILURE;

    // REGISTERING THE STOP SIGNAL (CTRL+C)
    signal(SIGINT, stopServer);
    
    // Create a ServerSocket the programm is listening to
    if (initConnection(port) == EXIT_FAILURE)
        return EXIT_FAILURE;
        
    if (registerCommands() == EXIT_FAILURE);
        return EXIT_FAILURE;

    printf("Terminal Server started at port %s.\n", port);

    serverLoop();

    return EXIT_SUCCESS;
}

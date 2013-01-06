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
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#include <poll.h>

#include "server.h"
#include "../common/network/network.h"

// ****************************************************
// This methodes are called when the server is starting
// ****************************************************
int init(int argc, char **args) {

    puts("Start Server...");
    char *port;
    puts("Parse Arguments from console...");
    if (parseArguments(argc, args, &port) == EXIT_FAILURE)
        return EXIT_FAILURE;

    puts("Initiating connection...");
    int serverSocket;
    if (initConnection(port, &serverSocket) == EXIT_FAILURE)
        return EXIT_FAILURE;

    printf("Gnuddels-Server started on the port %s!\n", port);

    return EXIT_SUCCESS;
}

int parseArguments(int argc, char **args, char **port) {
    // Not enough arguments
    if (argc < 3) {
        printf("Usage: %s -p Port\n", args[0]);
        return EXIT_FAILURE;
    }

    // Parse arguments
    int opt;
	while ((opt = getopt(argc, args, "p:")) != -1) {
		switch (opt) {
			case 'p':
                *port = optarg;
                break;
            default:
                fprintf(stderr, "Unknown paramater %c", opt);
                return EXIT_FAILURE;
		}
	}
	
	return EXIT_SUCCESS;
}

int initConnection(char *port, int *serverSocket) {
    
    // Information about the connection type
    struct addrinfo hints;
    memset(&hints, 0 , sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;        // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_flags = AI_PASSIVE;        // Listen to socket -> Allow every ip
    hints.ai_protocol = 0;              // Any protocol of TCP is allowed
    
    // Store information in struct "res"
    struct addrinfo *res;
    if (getaddrinfo(NULL, port, &hints, &res) == -1) {
        perror("Can't parse information for socket creation");
        return EXIT_FAILURE;
    }
    
    // Create Socket with the stored information
    *serverSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (*serverSocket < 0 ) {
        perror("Can't create new socket!");
        return EXIT_FAILURE;
    }

    // Flag for nonblocking
    int flag = 1;
    // Set socket to be nonblocking
    if (ioctl(*serverSocket, FIONBIO, (char *)&flag) < 0) {
        perror("ioctl() failed");
        close(*serverSocket);
        return EXIT_FAILURE;
    }
    
    // Make address of the server reusable (nice for debugging)
    if (setsockopt(*serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) < 0) {
        perror("setsockopt() failed");
        close(*serverSocket);
        return EXIT_FAILURE;
    }

    // Bind the Server to the Socket
    if (bind(*serverSocket, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Can't bind the server to the socket!");
        return EXIT_FAILURE;
    }

    // Free Memory
    freeaddrinfo(res);

    // Start Listening to the Socket    
    listen(*serverSocket, SOMAXCONN);

    return EXIT_SUCCESS;
}

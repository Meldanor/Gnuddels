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

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#include "server.h"
#include "../network/network.h"

int serverSocket;
static bool serverIsRunning = true;

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

int initConnection(char *port) {

    // Information about the connection type
    struct addrinfo hints;
    memset(&hints, 0 , sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // Listen to socket -> Allow every ip
    hints.ai_protocol = 0; // Any protocol of TCP is allowed
    
    // Store information in struct "res"
    struct addrinfo *res;
    if (getaddrinfo(NULL, port, &hints, &res) == -1) {
        perror("Can't parse information for socket creation");
        return EXIT_FAILURE;
    }
    
    // Create Socket with the stored information
    serverSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serverSocket < 0 ) {
        perror("Can't create new socket!");
        return EXIT_FAILURE;
    }
    // Bind the Server to the Socket
    if (bind(serverSocket, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Can't bind the server to the socket!");
        return EXIT_FAILURE;
    }
    // Free Memory
    freeaddrinfo(res);

    // Start Listening to the Socket    
    listen(serverSocket, SOMAXCONN);
    
    return EXIT_SUCCESS;
}

void serverLoop(void) {

    int clientSocket;
    socklen_t len = sizeof(struct sockaddr_in);

    // Main loop
    while (serverIsRunning) {
        // Struct for storing information about the client
        struct sockaddr_in *conInfo = malloc(sizeof (struct sockaddr_in));

        if (conInfo == NULL) {
            perror("Not enough memory!");
            break;
        }
        // BLOCKS UNTIL A CONNECTION IS INSIDE THE QUEUE
        clientSocket = accept(serverSocket, (struct sockaddr*)(conInfo), &len);
        if (clientSocket < 0 ) {
            perror("Can't accept a new client!");
            continue;
        }
        // Create new client and start handeling it
        addClient(clientSocket, conInfo);
        puts("New Client connected!");        
    }

    stopServer(EXIT_SUCCESS);
}

int addClient(int clientSocket, struct sockaddr_in *conInfo) {
    struct client *client = malloc(sizeof (struct client));
    // not enought memory
    if (client == NULL) {
        perror("Can't allocate memory for the clientData struct!");
        return EXIT_FAILURE;
    }
    // create in and out buffer for the client
    createClientStruct(client, clientSocket, conInfo);

    return EXIT_SUCCESS;
}

void handleClient(struct client *client) {

    // client loop
    while (client->isConnected) {
        // TODO: Implement chat logic    
    }
    // Free Memory
    freeClient(client);

    puts("Client disconnected");
}

void stopServer(int signal) {

    puts("Start server shutdown!");
    
    // CLOSE SERVER SOCKET
    close(serverSocket);
    puts("Closed server socket!");
    puts("Finished server shutdown!");
    exit(signal);
}

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

#include <pthread.h>

#include "server.h"
#include "clientData.h"
#include "../network/network.h"

#define MAX_CLIENTS 64

int serverSocket;
bool serverIsRunning = true;

static struct clientData *clients[MAX_CLIENTS] = {NULL};

int parseArguments(int argc, char **args, char *port) {
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
                *port = *optarg;
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
        struct sockaddr_in *connectionInformation = malloc(sizeof (struct sockaddr_in));
        if (connectionInformation == NULL) {
            perror("Not enough memory!");
            break;
        }
        // BLOCKS UNTIL A CONNECTION IS INSIDE THE QUEUE
        clientSocket = accept( serverSocket, (struct sockaddr*)(connectionInformation), &len);
        if (clientSocket < 0 ) {
            perror("Can't accept a new client!");
            continue;
        }
        // Create new client and start handeling it
        addClient(clientSocket, connectionInformation);
    }

    stopServer(EXIT_SUCCESS);
}

int addClient(int clientSocket, struct sockaddr_in *connectionInformation) {
    struct clientData *clientData = malloc(sizeof (struct clientData));
    // not enought memory
    if (clientData == NULL) {
        perror("Can't allocate memory for the clientData struct!");
        return EXIT_FAILURE;
    }
    int result = 0;
    // create data for the thread
    getClientData(clientData, clientSocket, connectionInformation);

    // Create thread
    pthread_t *thread = malloc(sizeof(pthread_t));
    if (thread == NULL) {
        perror("Can't allocate memory for thread!");
        return EXIT_FAILURE;
    }
    result = pthread_create(thread, NULL, &handleClient, clientData);
    if (result != 0) {
        perror("Can't create new thread for client!");
        return EXIT_FAILURE;
    }
    clientData->thread = thread;
    
    // Add client to list
    int i;
    for (i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] == NULL) {
            clients[i] = clientData;
            clientData->position = i;
            break;
        }
    }

    return EXIT_SUCCESS;
}

void *handleClient(void *arg) {

    puts("New Client connected!");
    struct clientData *clientData = (struct clientData *)(arg);

    // client loop
    while (clientData->isConnected) {
        // TODO: Implement chat logic    
    }
    // CLOSE CONNECTION
    close(clientData->clientSocket);
    // Free Memory
    clients[clientData->position] = NULL;
    clearClient(clientData);

    puts("Client disconnected");

    return NULL;
}

void stopServer(int signal) {

    puts("Start server shutdown!");

    int i;
    int counter= 0;
    for (i = 0 ; i < MAX_CLIENTS; ++i) {
        if (clients[i] != NULL) {
            close(clients[i]->clientSocket);
            clearClient(clients[i]);
            ++counter;
        }
    }
    printf("%d Clients disconnected!\n", counter);
    // CLOSE SERVER SOCKET
    close(serverSocket);
    puts("Closed server socket!");
    puts("Finished server shutdown!");
    exit(signal);
}

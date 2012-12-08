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
#include "../network/network.h"

int serverSocket;
static bool serverIsRunning = true;

static int clientCounter = 0;
static struct client **clients = NULL;

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

// Struct for poll
// Struct Array is always one bigger than there are connected clients(First Element is the ServerSocket)
static struct pollfd *fds;
// Current File Descriptors in poll
static int nfds;

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

    int on = 1;
    // Set socket to be nonblocking
    if (ioctl(serverSocket, FIONBIO, (char *)&on) < 0) {
        perror("ioctl() failed");
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

    // Init Poll Structure with one element
    // First element is always the server socket
    increasePollArray(serverSocket);
    return EXIT_SUCCESS;
}

void serverLoop(void) {

    socklen_t len = sizeof(struct sockaddr_in);
    int result = 0;
    // Main loop
    while (serverIsRunning) {
        result = poll(fds, nfds, -1);
        // Poll has failed
        if (result < 0) {
            perror("poll() failed!");
            break;
        }
        // Poll Timed out
        if (result == 0)
            continue;
        int i = 0;
        int curSize = nfds;
        // Look what registered FileDescriptor has is readable
        for (i = 0 ; i < curSize; ++i) {
            // FileDescriptor has no event
            if (fds[i].revents == 0)
                continue;
            // FileDescriptor has other event as registered!
            if (fds[i].revents != POLLIN) {
                serverIsRunning = false;
                break;
            }
            // Server socket has something to read -> new connection
            if (fds[i].fd == serverSocket) {
                // Accept all connections in the queue            
                // Do this until the error EWOULDBLOCK occurs
                do {
                    struct sockaddr_in *conInfo = malloc(sizeof (struct sockaddr_in));
                    if (conInfo == NULL) {
                        perror("Not enough memory!");
                        break;
                    }
                    // Pull new connection from connection stack
                    result = accept(serverSocket, (struct sockaddr*)(conInfo), &len);
                    if (result < 0) {
                        // Another error as EWOULDBLOCK happened
                        if (errno != EWOULDBLOCK) {
                            perror(" accept() failed!");
                            serverIsRunning = false;
                        }
                        break;
                    }
                    // Create new client
                    addClient(result,conInfo);
                } while (result != -1);
            }
            // Client want to send something
            else {
                // Search for the client who want to send something and handle it
                struct client *client = NULL;
                int j = 0;
                for (j = 0; j < clientCounter ; ++j) {
                    // Have found new client
                    if (clients[j]->socket == fds[i].fd) {
                        client = clients[j];
                        break;
                    }
                }
                if (client != NULL)
                    handleClient(client);
                else {
                    perror("Unknown FD in client array!");
                    break;
                }
            }
        }
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

    // Expand client list
    struct client **ptr = realloc(clients, (clientCounter + 1) * sizeof(struct client));
    if (ptr == NULL) {
        perror("Not enough memory!");
        return EXIT_FAILURE;
    }
    if (ptr != clients)
        clients = ptr;
        
    // Add client to list
    clients[clientCounter] = client;
    client->position = clientCounter;
    ++clientCounter;

    // Add new client socket to poll structure
    // Poll listens to all registered clients and need the sockets and events
    // Expand poll structure for one new client
    if (increasePollArray(clientSocket) == EXIT_FAILURE)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int increasePollArray(int newFileDescriptor) {
    struct pollfd *ptr = realloc(fds, (nfds + 1) * sizeof(struct pollfd));
    // Not enough memory!
    if (ptr == NULL) {
        perror(" not enough memory to increase poll array!");
        return EXIT_FAILURE;     
    }
    // New location in Memory
    if (ptr != fds)
        fds = ptr;

    fds[nfds].fd = newFileDescriptor;
    // Wait for Incoming Data
    fds[nfds].events = POLLIN;
    ++nfds;

    return EXIT_SUCCESS;
}

// Complete server logic
void handleClient(struct client *client) {

    // Size of message from client
    int recBytes = 0;
    // Read the complete message from client
    if (readFromClient(client, &recBytes) == EXIT_FAILURE) {
        removeClient(client);
        return;
    }
}

int readFromClient(struct client *client, int *recBytes) {
    int curRecBytes;
    int recBytesTotal;
    do {
        // Read until the error WOULD BLOCk occurs(poll strategy)
        curRecBytes = recv(client->socket, (client->inBuffer) + recBytesTotal, (client->inBufferSize) - recBytesTotal, 0);
        // Error occured
        if (curRecBytes < 0) {
            // Unexpeteced error -> disconnect client
            if (errno != EWOULDBLOCK) {
                perror (" recv() failed!");
                return EXIT_FAILURE;
            }
            break;
        }
        recBytesTotal += curRecBytes;
        client->inBuffer[recBytesTotal] = '\0';
        // buffer to low!
        if (recBytesTotal == client->inBufferSize) {
            // TODO: Handle buffer overflow
            return EXIT_FAILURE;
        }
    } while(true);

    // Write total read byte count to the parameter
    *recBytes = recBytesTotal;
    return EXIT_SUCCESS;
}

void removeClient(struct client *client) {
    // Swap last position with current to free position
    clients[client->position] = clients[clientCounter - 1];
    clients[clientCounter - 1]->position = client->position;
    clients[clientCounter - 1] = NULL;

    // reduce client list
    clients = realloc(clients, (clientCounter - 1) * sizeof(struct client));
    --clientCounter;
    
    // Reduce Poll Structure
    int i;
    // Search for matching poll structure
    for (i = 1 ; i < nfds ; ++i) {
        if (fds[i].fd == client->socket)
            break;
    }
    if (i == nfds) {
        perror("Unknown FD in client array!");
    }
    else {
        // Swap last position with fd to delete
        fds[i] = fds[nfds-1];
        
        // Reduce Poll Array by one element(the last one)
        struct pollfd *ptr = realloc(fds, (nfds - 1) * sizeof(struct pollfd));
        // Not enough memory!
        if (ptr == NULL) {
            perror(" not enough memory to decrease poll array!");
            return;
        }
        // New location in Memory
        if (ptr != fds)
            fds = ptr;
    }
    // Free Memory
    freeClient(client);
}

void stopServer(int signal) {

    puts("Start server shutdown!");

    printf("Disconnect %d Clients", clientCounter);
    int i;
    for (i = 0; i < clientCounter ; ++i) {
        freeClient(clients[i]);
    }
    free(clients);
    // CLOSE SERVER SOCKET
    close(serverSocket);
    puts("Closed server socket!");
    puts("Finished server shutdown!");
    exit(signal);
}

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
#include "../common/StringBuffer.h"
#include "../common/datatype/GenericVector.h"

// *******************************************************
// Methods only called when server is starting / stopping
// *******************************************************

static int serverSocket;

int init(int argc, char **args) {

    puts("Start Server...");
    char *port;
    puts("Parse Arguments from console...");
    if (parseArguments(argc, args, &port) == EXIT_FAILURE)
        return EXIT_FAILURE;

    puts("Initiating connection...");
    if (initConnection(port) == EXIT_FAILURE)
        return EXIT_FAILURE;
        
    if (initPoll() == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
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

int initConnection(char *port) {
    
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
    serverSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serverSocket < 0 ) {
        perror("Can't create new socket!");
        return EXIT_FAILURE;
    }

    // Flag for nonblocking
    int flag = 1;
    // Set socket to be nonblocking
    if (ioctl(serverSocket, FIONBIO, (char *)&flag) < 0) {
        perror("ioctl() failed");
        close(serverSocket);
        return EXIT_FAILURE;
    }
    
    // Make address of the server reusable (nice for debugging)
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) < 0) {
        perror("setsockopt() failed");
        close(serverSocket);
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

DefVector(struct pollfd, poll);
static pollVector *pollList;

int
initPoll() {
    // init list for poll structs
    pollList = pollVector_construct(1);
    if (pollList == NULL) {
        return EXIT_FAILURE;
    }
    // add pollstruct to list
    struct pollfd serverPollfd;
    serverPollfd.fd = serverSocket;
    serverPollfd.events = POLLIN;    
    pollVector_add(pollList, serverPollfd);
    
    return EXIT_SUCCESS;
}

void stopServer(void) {

    puts("Stopping server...");
    puts("Close socket...");
    close(serverSocket);
}

// *******************************************
// Methods called when the server is running
// *******************************************

#define INFINITE_TIMEOUT -1

#define CLIENT_DISCONNECTED -2 

void
serverLoop(void) {

    while (true) {
        // res stores the numbers of file descriptors throwed an event
        int res = poll(pollList->elements, pollList->size, INFINITE_TIMEOUT);
        // Poll returns without any events
        if (res == 0) {
            continue;
        }
        if (res < 0) {
            perror("poll failed!");
            break;
        }
        int i;
        struct pollfd *pollfd = NULL;
        for (i = 0 ; i < pollList->size; ++i) {
            pollfd = pollVector_get(pollList, i);
            // This fd didn't fired an event
            if (pollfd->revents == 0) {
                continue;
            }
            // The fd want to send something
            if ((pollfd->revents & POLLIN) == POLLIN) {
                // New client want to connect
                if (pollfd->fd == serverSocket) {
                    // try to accept new client
                    if (accept_newClient() == EXIT_FAILURE) {
                        return;
                    }
                }
                // Connected client want to send something
                else {
                    // Read it's input and handle it
                    int cRes = handle_client(pollfd->fd);
                    // Client disconnected
                    if (cRes == CLIENT_DISCONNECTED) {
                        printf("Client %d disconnected\n", pollfd->fd);
                        remove_client(pollfd->fd);
                        --i;
                    }
                    // An error occurred
                    else if (cRes == EXIT_FAILURE) {
                        printf("Client %d crashed!\n", pollfd->fd);
                        remove_client(pollfd->fd);
                        --i;
                    }
                    else {
                        // Do Nothing
                    }
                }
            }
            // Unregistered poll event was thrown
            else {
                fprintf(stderr, "Unkown poll event %d! Server is stopping", pollfd->revents);
                return;
            }
        }
    }
}

int
accept_newClient() {

    // Accept new clients in the connection queue
    while (1) {
        // Get one single client from the queue
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket < 0) {
            if (errno == EWOULDBLOCK) {
                break;
            }
            perror("accept failed!");
            return EXIT_FAILURE;
        }

        // Flag for nonblocking
        int flag = 1;
        // Set socket to be nonblocking
        if (ioctl(clientSocket, FIONBIO, (char *)&flag) < 0) {
            perror("ioctl() failed");
            close(clientSocket);
            return EXIT_FAILURE;
        }

        // Add client to pollList
        struct pollfd pollfd;
        pollfd.fd = clientSocket;
        pollfd.events = POLLIN;
        pollVector_add(pollList, pollfd);

        printf("Client %d connected\n", clientSocket);
    }
    return EXIT_SUCCESS;
}

int equals_pollfd(struct pollfd *fd1, struct pollfd *fd2) {
    return (fd1->fd == fd2->fd ? 0 : -1);
}

int
remove_client(int socket) {

    // Close the socket
    close(socket);
    struct pollfd temp;
    temp.fd = socket;
    // Remove it from the poll list
    pollVector_remove(pollList, &temp, &equals_pollfd);
    return EXIT_SUCCESS;
}

int
handle_client(int socket) {
    // TODO: Improve buffer management
    char buffer[4096] = {0};
    while(1) {
        int bytes_read = read(socket, buffer, 4096);
        if (bytes_read == 0) {
            return CLIENT_DISCONNECTED;
        }
        if (bytes_read < 0) {
            if (errno == EWOULDBLOCK) {
                return EXIT_SUCCESS;
            }
            perror("read failed!");
            return EXIT_FAILURE;
        }
        printf("Client %d sent something\n", socket);
    }

    return EXIT_SUCCESS;
}

int
read_from_client(int socket) {
    // TODO: Implement reading from a client
    return EXIT_SUCCESS;
}


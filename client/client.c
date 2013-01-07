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
 
#include "client.h"
#include "../common/network/network.h"
#include "../common/StringBuffer.h"
#include "../common/datatype/GenericVector.h"

// *******************************************************
// Methods only called when server is starting / stopping
// *******************************************************

static int clientSocket;
static int infd;
static int outfd;

int initClient(int argc, char **args, int infd_, int outfd_) {

    puts("Start Client...");
    
    infd = infd_;
    outfd = outfd_;
    
    char *port;
    char *host;
    puts("Parse Arguments from console...");
    if (parseArguments(argc, args, &host, &port) == EXIT_FAILURE)
        return EXIT_FAILURE;

    printf("Initiating connection on host %s on port %s...\n", host, port);
    if (initConnection(host, port) == EXIT_FAILURE)
        return EXIT_FAILURE;
        
    if (initPoll() == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    printf("Gnuddels-Client connected to %s on port %s!\n", host, port);

    return EXIT_SUCCESS;
}

int parseArguments(int argc, char **args, char **host, char **port) {
    // Not enough arguments
    if (argc < 5) {
        printf("Usage: %s -p [Port] -h [Host]\n", args[0]);
        return EXIT_FAILURE;
    }

    // Parse arguments
    int opt;
	while ((opt = getopt(argc, args, "p:h:")) != -1) {
		switch (opt) {
			case 'p':
                *port = optarg;
                break;
            case 'h':
                *host = optarg;
                break;
            default:
                fprintf(stderr, "Unknown paramater %c", opt);
                return EXIT_FAILURE;
		}
	}
	
	return EXIT_SUCCESS;
}

int initConnection(char *host, char *port) {
    
    // Information about the connection type
    struct addrinfo hints;
    memset(&hints, 0 , sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;        // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_flags = 0;                 // No flag set
    hints.ai_protocol = 0;              // Any protocol of TCP is allowed
    
    // Store information in struct "res"
    struct addrinfo *res;
    if (getaddrinfo(host, port, &hints, &res) == -1) {
        perror("Can't parse information for socket creation");
        return EXIT_FAILURE;
    }
    
    // Create Socket with the stored information
    clientSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (clientSocket < 0 ) {
        perror("Can't create new socket!");
        return EXIT_FAILURE;
    }
/*
    // Flag for nonblocking
    int flag = 1;
    // Set socket to be nonblocking
    if (ioctl(clientSocket, FIONBIO, (char *)&flag) < 0) {
        perror("ioctl() failed");
        close(clientSocket);
        return EXIT_FAILURE;
    }
  */  
    if (connect(clientSocket, res->ai_addr, res->ai_addrlen) < 0) {
        //if (errno != EINPROGRESS) {
            perror("connect() failed");
            return EXIT_FAILURE;
        //}
    }
    // Free Memory
    freeaddrinfo(res);

    return EXIT_SUCCESS;
}

DefVector(struct pollfd, poll);
static pollVector *pollList;

int
initPoll(void) {
    // init list for poll structs
    pollList = pollVector_construct(1);
    if (pollList == NULL) {
        return EXIT_FAILURE;
    }
    // add pollstruct to list
    struct pollfd clientPollFD;
    clientPollFD.fd = clientSocket;
    clientPollFD.events = POLLIN;    
    pollVector_add(pollList, clientPollFD);
    
    struct pollfd GUIInputFD;
    GUIInputFD.fd = infd;
    GUIInputFD.events = POLLIN;    
    pollVector_add(pollList, GUIInputFD);
    
    return EXIT_SUCCESS;
}

void stopClient(void) {

    puts("Stopping client...");
    puts("Close socket...");
    close(clientSocket);
}

// ***************************************
// Methods called while client is running
// ***************************************

#define INFINITE_TIMEOUT -1

void clientLoop(void) {

    bool clientIsRunning = true;    
    while (clientIsRunning) {
        // res stores the numbers of file descriptors throwed an event
        int res = poll(pollList->elements, pollList->size, INFINITE_TIMEOUT);
        // Poll returns without any events
        if (res == 0) {
            continue;
        }
        if (res < 0) {
            perror("poll failed!");
            clientIsRunning = false;
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
                // User has typed some text into the GUI
                if (pollfd->fd == infd) {
                    if (read_from_gui() == EXIT_FAILURE) {
                        clientIsRunning = false;
                        break;
                    }
                }
                // Server has sent something to the client
                else if(pollfd->fd == clientSocket) {
                    if (read_from_server() == EXIT_FAILURE) {
                        clientIsRunning = false;
                        break;
                    }
                }
                // Unregistered FD sent something
                else {
                    perror("Unknown FD fired event!");
                    clientIsRunning = false;
                    break;
                }
            }
            // Unregistered poll event was thrown
            else {
                fprintf(stderr, "Unkown poll event %d! Client is stopping\n", pollfd->revents);
                return;
            }
        }
    }
}
#define IN_BUFFER_SIZE 4096
static char inBuffer[IN_BUFFER_SIZE + 1] = {0};

int read_from_server(void ) {
    //while(1) {
        int bytes_read = read(clientSocket, inBuffer, IN_BUFFER_SIZE);
        if (bytes_read == 0) {
            perror("server closed connection!");
            return EXIT_FAILURE;
        }
        if (bytes_read < 0) {
        /*    if (errno == EWOULDBLOCK) {
                return EXIT_SUCCESS;
            }*/
            perror("read failed!");
            return EXIT_FAILURE;
        }
        // Copy received message to the client buffer
        write(outfd, inBuffer, bytes_read);
        
    //}
    
    return EXIT_SUCCESS;
}

int read_from_gui(void) {
    puts("Gui sent something...");
    int bytes_read = read(infd, inBuffer, IN_BUFFER_SIZE);
    if (bytes_read == 0) {
        puts("Gui closed...");
        return EXIT_FAILURE;
    }
    if (bytes_read < 0) {
        perror("read failed!");
        return EXIT_FAILURE;
    }
    sendAll(clientSocket, inBuffer, bytes_read);
} 

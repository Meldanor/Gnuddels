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
#include <arpa/inet.h>

#include <poll.h>

#include "server.h"
#include "../common/network/network.h"
#include "../common/StringBuffer.h"
#include "../common/datatype/GenericVector.h"

// *******************************************************
// Methods only called when server is starting / stopping
// *******************************************************

// File Descriptor to the serverSocket(accepting new connections)
static int serverSocket;

// Create Generic Vector storing pollfd
DefVector(struct pollfd, poll);
// List storing pollfd for poll()
static pollVector *pollList;

DefVector(Client, client);

static clientVector *clientList;

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
    
    clientList = clientVector_construct(8);
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
                fprintf(stderr, "Unkown poll event %d! Server is stopping\n", pollfd->revents);
                return;
            }
        }
    }
}

int
accept_newClient() {

    // Accept new clients in the connection queue
    while (1) {
        struct sockaddr_in conInfo;
        socklen_t conInfo_len = sizeof(struct sockaddr_in);
    
        // Get one single client from the queue
        int clientSocket = accept(serverSocket, (struct sockaddr*)(&conInfo), &conInfo_len);
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
        
        // Add client to clientList
        // Convert client address to readable IP4 formatted string
        // This is the standard name of all new users
	    char *ip = inet_ntoa(conInfo.sin_addr);
        Client *client = Client_construct(clientSocket, ip);
        clientVector_add(clientList, *client);

        printf("Client %s connected\n", ip);
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
    
    // Remove registered socket from poll list
    struct pollfd tempPollfd;
    tempPollfd.fd = socket;
    pollVector_remove(pollList, &tempPollfd, &equals_pollfd);
    
    // Remove registered Client from client list
    Client tempClient;
    tempClient.socket = socket;
    clientVector_remove(clientList, &tempClient, &equals_Client_Socket);
    return EXIT_SUCCESS;
}

#define MSG_DELIMITER '\n'

int
handle_client(int socket) {

    // Get the client registered to the socket
    Client *client = search_client(socket);
    if (client == NULL) {
        perror("Unregistered client socket found! This is not in the list!");
        return EXIT_FAILURE;
    }
    // Read the complete input of the client
    int res = read_from_client(client);
    if (res != EXIT_SUCCESS)
        return res;
        
    // Try to extract a single message from the buffer
    StringBuffer *msg = extract_message(client);
    if (msg == NULL) {
        return EXIT_SUCCESS;
    }
    if (is_command(client, msg) == EXIT_SUCCESS) {
        handle_command(client, msg);
    }
    else {
        broadcast_message(client, msg);
    }
    
    StringBuffer_free(msg);
    return EXIT_SUCCESS;
}

// ***********************************
// Methods for client input handeling
// ***********************************

StringBuffer
*extract_message(Client *client) {
    // If MSG_DELIMITER is part of the string, the client has sendet a complete message
    char *subString = strchr(client->buffer->buffer, MSG_DELIMITER);
    if (subString == NULL) {
        return NULL;
    }
    // Extract single message from buffer
    // Calculate the length of the message
    int len = subString - client->buffer->buffer;
    // Copy mesage to temponary buffer
    StringBuffer *msg = StringBuffer_construct_n(len);
    StringBuffer_concat_n(msg, client->buffer->buffer, len);
    
    // Delete the leading \n
    subString = subString + 1;
    // Regorganize buffer by moving the rest of the buffer without the message to the beginning of the buffer
    memmove(client->buffer->buffer, subString, client->buffer->size - (len + 1));
    client->buffer->size = client->buffer->size - (len + 1);
    client->buffer->buffer[client->buffer->size] = '\0';
    return msg;
}

Client
*search_client(int socket) {
    Client *client = NULL;    
    int i;
    for (i = 0 ; i < clientList->size; ++i) {
        if (clientList->elements[i].socket == socket) {
            client = &(clientList->elements[i]);
            break;
        }
    }
    return client;
}

#define IN_BUFFER_SIZE 4096
static char inBuffer[IN_BUFFER_SIZE + 1] = {0};

int
read_from_client(Client *client) {
    while(1) {
        int bytes_read = read(client->socket, inBuffer, IN_BUFFER_SIZE);
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
        // Copy received message to the client buffer
        StringBuffer_concat_n(client->buffer, inBuffer, bytes_read);
    }
    
    return EXIT_SUCCESS;
}

int broadcast_message(Client *client, StringBuffer *msg) {

    // Construct message
    StringBuffer *temp = StringBuffer_construct_n(msg->size);
    StringBuffer_concat(temp, "[");
    StringBuffer_concat(temp, client->name);
    StringBuffer_concat(temp, "]: ");
    StringBuffer_concat(temp, msg->buffer);
    
    // Send message to all clients
    Client *receiver = NULL;
    int i;
    for(i = 0 ; i < clientList->size; ++i) {
        receiver = clientVector_get(clientList, i);
        sendAll(receiver->socket, temp->buffer, temp->size);
    }
    
    StringBuffer_free(temp);
    return EXIT_SUCCESS;
}

#define COMMAND_START '/'

int is_command(Client *client, StringBuffer *msg) {
    return msg->buffer[0] == COMMAND_START ? EXIT_SUCCESS : EXIT_FAILURE;
}

int handle_command(Client *client, StringBuffer *msg) {

    // Skip the COMMAND_START
    char *syntax = msg->buffer + 1;
    // Extract the args of the command
    char *args = strchr(syntax, ' ');
    int syn_len = 0;

    // Store the args in command buffer
    StringBuffer *command = NULL;
    // When command has arguments
    if (args != NULL) {

        syn_len = args - syntax;        
        command = StringBuffer_construct();
        StringBuffer_concat(command, args + 1);
    }
    else {
        syn_len = msg->size;
    }
    // List command
    if (strncmp(syntax, "list", syn_len) == 0) {
        command_list(client, command);   
    }
    // Nick command
    else if (strncmp(syntax, "nick", syn_len) == 0) {
        command_nick(client, command);   
    }
    // Message command
    else if (strncmp(syntax, "msg", syn_len) == 0) {
        command_msg(client, command);   
    }
    // Unknown command!
    else {
        StringBuffer *errorMsg = StringBuffer_construct();
        StringBuffer_concat(errorMsg, "ERROR: Unbekannter Befehl '");
        StringBuffer_concat(errorMsg, syntax);
        StringBuffer_concat(errorMsg, "'!");
        sendAll(client->socket, errorMsg->buffer, errorMsg->size);
        StringBuffer_free(errorMsg);
    }

    // Free temp memory
    if (command != NULL)
        StringBuffer_free(command);
    return EXIT_SUCCESS;
}

// ***********************************
// Command methods
// ***********************************

int command_list(Client *client, StringBuffer *command) {
    StringBuffer *msg = StringBuffer_construct();
    StringBuffer_concat(msg, "Verbundene Clients(");
    char temp[15];
    sprintf(temp, "%d", clientList->size);
    StringBuffer_concat(msg, temp);
    StringBuffer_concat(msg, ")\n");
    int i = 0;
    // Build message
    for (i = 0; i < clientList->size - 1; ++i) {
        StringBuffer_concat(msg, "[");     
        StringBuffer_concat(msg, clientVector_get(clientList, i)->name);
        StringBuffer_concat(msg, "]");
        StringBuffer_concat(msg, ", ");
    }
    // Add last one without ,
    StringBuffer_concat(msg, "[");     
    StringBuffer_concat(msg, clientVector_get(clientList, i)->name);
    StringBuffer_concat(msg, "]");
    
    // Send message to client
    sendAll(client->socket, msg->buffer, msg->size);

    StringBuffer_free(msg);
    
    return EXIT_SUCCESS;
}

int command_nick(Client *client, StringBuffer *command) {

    StringBuffer *msg = StringBuffer_construct();
    // No arguments
    if (command == NULL) {
        StringBuffer_concat(msg, "ERROR: Keinen Nicknamen angegeben!");
        sendAll(client->socket, msg->buffer, msg->size);
        StringBuffer_free(msg);
        return EXIT_FAILURE;
    }
    Client temp;
    temp.name = command->buffer;    
    // Search for double names
    if (clientVector_contains(clientList, &temp, &equals_Client_Name) == EXIT_SUCCESS) {
        StringBuffer_concat(msg, "ERROR: Es existiert bereits ein Client namens '");
        StringBuffer_concat(msg, command->buffer);
        StringBuffer_concat(msg, "'!");
        sendAll(client->socket, msg->buffer, msg->size);
        StringBuffer_free(msg);
        return EXIT_FAILURE;
    }

    StringBuffer_concat(msg, "INFO: '");
    StringBuffer_concat(msg, client->name);
    StringBuffer_concat(msg, "' nennt sich nun '");
    StringBuffer_concat(msg, command->buffer);
    StringBuffer_concat(msg, "'.");
    
    client->name = strdup(command->buffer);
    
    // Send message to all clients
    Client *receiver = NULL;
    int i;
    for(i = 0 ; i < clientList->size; ++i) {
        receiver = clientVector_get(clientList, i);
        sendAll(receiver->socket, msg->buffer, msg->size);
    }
    
    StringBuffer_free(msg);
    return EXIT_SUCCESS;
}

int command_msg(Client *client, StringBuffer *command) {

    // No nickname in args
    if (command == NULL) {
        StringBuffer *errMsg = StringBuffer_construct();
        StringBuffer_concat(errMsg, "ERROR: Keinen Nicknamen angegeben!");
        sendAll(client->socket, errMsg->buffer, errMsg->size);
        StringBuffer_free(errMsg);
        return EXIT_FAILURE;
    }
    // Look if there is an message to whisper
    char *whisperText = strchr(command->buffer, ' ');
    if (whisperText == NULL) {
        StringBuffer *errMsg = StringBuffer_construct();
        StringBuffer_concat(errMsg, "Keine Nachricht angegeben!");
        sendAll(client->socket, errMsg->buffer, errMsg->size);
        StringBuffer_free(errMsg);
        return EXIT_FAILURE;
    }
    // Split string at position
    *whisperText = '\0';
    whisperText = whisperText + 1;
    // Looking for receiver
    Client temp;
    temp.name = command->buffer;
    Client *receiver = NULL;
    int i = 0;
    for (i = 0 ; i < clientList->size; ++i) {
        receiver = clientVector_get(clientList, i);
        // Check if names are equals
        if (equals_Client_Name(&temp, receiver) == 0) {
            break;
        }
        else {
            receiver = NULL;
        }
    }
    // Receiver not found
    if (receiver == NULL) {
        StringBuffer *errMsg = StringBuffer_construct();
        StringBuffer_concat(errMsg, "ERROR: Client '");
        StringBuffer_concat(errMsg, command->buffer);
        StringBuffer_concat(errMsg, "' ist nicht online!");
        sendAll(client->socket, errMsg->buffer, errMsg->size);
        StringBuffer_free(errMsg);
        return EXIT_FAILURE;
    }

    // Build message for caller
    // Whisper message format: [me -> RECEIVER]: MESSAGE
    StringBuffer *msg = StringBuffer_construct();
    StringBuffer_concat(msg, "[me -> ");
    StringBuffer_concat(msg, receiver->name);
    StringBuffer_concat(msg, "]: ");
    StringBuffer_concat(msg, whisperText);
    sendAll(client->socket, msg->buffer, msg->size);
    StringBuffer_clear(msg);
    
    // Build message for receiver
    // Whisper message format: [CALLER -> me]: MESSAGE
    StringBuffer_concat(msg, "[");
    StringBuffer_concat(msg, client->name);
    StringBuffer_concat(msg, " -> me]: ");
    StringBuffer_concat(msg, whisperText);
    sendAll(receiver->socket, msg->buffer, msg->size);
    StringBuffer_free(msg);

    return EXIT_SUCCESS;
}

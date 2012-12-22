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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CommandHandler.h"

bool isCommand(char *message) {
    return message[0] == '/';
}

struct RegisteredCommand {
    const char *prefix;
    bool (*execute)(struct Command *);
};

struct RegisteredCommand **commands = NULL;
int commandCount = 0;

bool registerCommand(const char *prefix, bool (*execute)(struct Command *)) {
    // Create new Command
    struct RegisteredCommand *newCommand = malloc(sizeof (struct RegisteredCommand));
    if (newCommand == NULL) {
        perror(" malloc() for struct Command failed!");
        return false;
    }
    newCommand->prefix = prefix;
    newCommand->execute = execute;

    // Expand command list
    struct RegisteredCommand **ptr = realloc(commands, sizeof(struct RegisteredCommand) * (commandCount + 1));
    if (ptr == NULL) {
        perror(" realloc() for registered Commands failed!");
        return false;
    }
    if (ptr != commands)
        commands = ptr;

    // Add command
    commands[commandCount++] = newCommand;
    return true;
}

bool executeCommand(const char *msg, struct client *caller) {

    // Split message to single arguments
    char **args = NULL;
    // Create write-able copy
    char *copy = strdup(msg);    
    char *ch = strtok(copy, " ");
    
    int argc = 0;
    // Split string
    while (ch != NULL) {
        char **ptr = realloc(args, (sizeof(char*)) * (argc + 1));
        if (ptr == NULL) {
            perror ("Memory null");
            return EXIT_FAILURE;
        }
        if (ptr != args)
            args = ptr;
        // Copy token to argument list
        args[argc] = strdup(ch);
        ++argc;
        ch = strtok(NULL, " ");
    }

    free(copy);
    
    // String is empty
    if (args == 0) {
        perror ("Argument count is null!");
        return false;
    }

    char *prefix = args[0];

    // Search for match in registered commands
    int i;
    bool found = false;
    for (i = 0; i < commandCount; ++i) {
        // Search for same prefix
        if (strcmp(prefix, commands[i]->prefix) == 0) {
            found = true;

            // Encapsulate the command
            struct Command command;
            command.argsSize = argc - 1;
            command.args = (args + 1);
            command.caller = caller;
            commands[i]->execute(&command);
            break;
        }
    }
    // Free string duplicates
    for (i = 0 ; i < argc; ++i)
        free(args[i]);

    return found;
}

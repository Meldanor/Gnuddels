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

#include <string.h>
#include "CommandHandler.h"


bool isCommand(char *message) {
    return message[0] == '/';
}

bool executeCommand(char *message) {
 
    // TODO: implement searching for the command and execute it
    return false;
}

bool registerCommand(char *prefix, bool (*execute)(int, char **)) {

    // TODO: Implement adding the command to an intern list
    return false;
}

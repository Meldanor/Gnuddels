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

#include <unistd.h>
#include <stdio.h>
#include <chatgui.h>

#include "client/client.h"

#define BUFSIZE 1024


int main(int argc, char *args[]) {
    // Start 
    // (C) 2012 Timo Lindhorst <lindhors@ivs.cs.ovgu.de>  
    // Copied from original 
    
	// Pass these to gui_start() and use them instead of stdin/stdout.
	int infd, outfd;

	char buf[BUFSIZE];

	// start GUI
	if((gui_start(&infd, &outfd)) < 0) {
		fprintf(stderr, "Failed to start GUI -- exiting\n");
		return -1;
	}

	while (1) {
		int len = read(infd, buf, BUFSIZE);  // read input from GUI
		if (len <= 0) {
			printf("GUI closed\n");
			break;
		}
		write(outfd, buf, len);	 // write output to GUI
	}
    // End
    // (C) 2012 Timo Lindhorst <lindhors@ivs.cs.ovgu.de>  
    // Copied from original 
    
	return 0;
}

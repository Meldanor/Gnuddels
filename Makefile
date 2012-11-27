# Makefile for echoclient

ARCH64=''
ifeq (64,$(findstring 64,$(shell uname -m)))
	ARCH64=64
endif

CC	= gcc
CFLAGS  += -Wall -g -Iinclude
LDFLAGS += -Llib
LDLIBS  += -lcnaiapi$(ARCH64) -lchatgui$(ARCH64) -lpthread

GTKCFLAGS=$(shell pkg-config --cflags gtk+-2.0)
GTKLDLIBS=$(shell pkg-config --libs gtk+-2.0)

CLIENT = $(CLIENT_NAME)-$(CLIENT_VERSION)
CLIENT_NAME = GnuddelsClient
CLIENT_VERSION = 0.0.1-SNAPSHOT

SERVER = $(SERVER_NAME)-$(SERVER_VERSION)
SERVER_NAME = GnuddelsServer
SERVER_VERSION = 0.0.1-SNAPSHOT

APPS = $(CLIENT) $(SERVER)

apps: createBuildDir $(APPS) cleanBuild

createBuildDir:
	mkdir -p bin

$(CLIENT): chatclient.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) $(GTKLDLIBS) -o bin/$(CLIENT) client/*.c

chatgui.o: chatgui.c
	$(CC) -c $(CFLAGS) $(GTKCFLAGS) -o $(SERVER) $^
	
$(SERVER): chatserver.c
	$(CC) chatserver.c -Wall -Iinclude -pthread -o bin/$@ server/*.c

.PHONY: clean

clean:
	rm -rf *.o $(APPS)

cleanBuild:
	rm chatclient.o

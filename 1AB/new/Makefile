CC=gcc
CFLAGS=-c -Wall
LINKFLAGS= -pthread

all: client server

server: udp_chat_server.o unp_readline.o
	$(CC) udp_chat_server.o unp_readline.o -o udp_chat_server

client: udp_chat_client.o unp_readline.o
	$(CC) $(LINKFLAGS) udp_chat_client.o unp_readline.o -o udp_chat_client

udp_chat_server.o: udp_chat_server.c
	$(CC) $(CFLAGS) udp_chat_server.c

udp_chat_client.o: udp_chat_client.c udp_chat_client.h
	$(CC) $(CFLAGS) udp_chat_client.c

unp_readline.o: unp_readline.c unp_readline.h
	$(CC) $(CFLAGS) unp_readline.c

clean:
	rm -rf *.o udp_chat_server udp_chat_client

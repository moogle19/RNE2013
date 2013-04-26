#ifndef UDP_CHAT_CLIENT_CONNECT_H
#define UDP_CHAT_CLIENT_CONNECT_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>


int connectToSock(char*,int,char*);
int printError();

#endif /* UDP_CHAT_CLIENT_CONNECT_H */

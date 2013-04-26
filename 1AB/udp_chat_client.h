#ifndef UDP_CHAT_CLIENT_H
#define UDP_CHAT_CLIENT_H


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "unp_readline.h"

int main(int, char**);
void printUsage();
int checkusername(char*);
int checkport(char*);
int checkip(char*);

#endif /* UDP_CHAT_CLIENT_H */

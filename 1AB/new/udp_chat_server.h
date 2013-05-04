#ifndef UDP_CHAT_SERVER_H
#define UDP_CHAT_SERVER_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include "unp_readline.h"

extern const char ID_CL_CON_REQ;
extern const char ID_SV_CON_REP;
extern const char ID_SV_CON_AMSG;
extern const char ID_CL_MSG;
extern const char ID_SV_AMSG;
extern const char ID_CL_DISC_REQ;
extern const char ID_SV_DISC_REP;
extern const char ID_SV_DISC_AMSG;
extern const char ID_SV_PING_REQ;
extern const char ID_CL_PING_REP;
extern const char ID_SV_MSG;

struct client
{
	struct sockaddr_in clientaddr;
	char* username;
    int clientsock;
	struct client* next;
};

int main(int, char**);
void handleSigInt(int);

int setPort(char*, struct sockaddr_in*);

//inputcheck
int checkUsername(char*);

//client handling
int addClient(char*, struct sockaddr_in, int);
int removeClient();

//messaging
void broadcastUserMessage(char*, int);
void broadcastMessage();
void broadcastDisconnectMessage();

int cleanClients();

void printUsage();



#endif /* UDP_CHAT_SERVER_H */
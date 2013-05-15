#ifndef UDP_CHAT_CLIENT_H
#define UDP_CHAT_CLIENT_H

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

int main(int, char**);
void handleSigInt(int);

//inputcheck
int checkUsername(char*, char*);
int checkPort(char*, struct sockaddr_in*);
int checkIp(char*, struct sockaddr_in*);

//socket
int getClientsock();

//server connection
int connectToServer(struct sockaddr_in* , char*, int);
int sendConnectionRequest(int, struct sockaddr_in*, char*);

//messaging
void parseRecBuffer(uint8_t*, int, struct sockaddr_in);
void printUserMessage(uint8_t*);
void printMessage(uint8_t*);
void printServerMessage(uint8_t*);
void printDisconnectMessage(uint8_t*);
void printServerDisconnect(uint8_t*);

int sendMessage(char*, struct sockaddr_in, int);

void sendPing(struct sockaddr_in, int);

int disconnectFromServer(struct sockaddr_in*, int);
int sendDisconnect(struct sockaddr_in*, int);

void printUsage();

#endif /* UDP_CHAT_CLIENT_H */

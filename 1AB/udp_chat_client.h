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
void printUsage();
int checkusername(char*, char*);
int checkport(char*);
int checkip(char*);
int getClientsock();
int connectToSock(char*,int,char*, int);
char* getClconreq(char*);
void getUserMessage(uint8_t*);
void getServerMessage(uint8_t*);
void printRecBuffer(uint8_t*);
void sendMessage(char*, int, struct sockaddr_in);
void getMessage(uint8_t*);
void* messageThread(void*);
uint16_t checkClconreq(uint8_t*);
void setClconreq(uint8_t*,uint16_t,char*);
int printError();


#endif /* UDP_CHAT_CLIENT_H */

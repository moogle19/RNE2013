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









int connectToSock(char*,int,char*);
char* getClconreq(char*);
uint16_t checkClconreq(char*);
int printError();

#endif /* UDP_CHAT_CLIENT_CONNECT_H */

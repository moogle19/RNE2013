#include "udp_chat_server.h"

//Ids of all messages
const char ID_CL_CON_REQ = 1;
const char ID_SV_CON_REP = 2;
const char ID_SV_CON_AMSG = 3;
const char ID_CL_MSG = 4;
const char ID_SV_AMSG = 5;
const char ID_CL_DISC_REQ = 6;
const char ID_SV_DISC_REP = 7;
const char ID_SV_DISC_AMSG = 8;
const char ID_SV_PING_REQ = 9;
const char ID_CL_PING_REP = 10;
const char ID_SV_MSG = 11;

int gContinue = 1;
int clientcount = 0;
struct client *firstclient;
struct client *currentclient;

void handleSigInt(int param) {
	gContinue = 0;
}

int main(int argc, char** argv) {
	
	int err = 0;
	int progargs;
	int serversock;
	extern char *optarg;
	extern int optind, optopt;
	socklen_t flen = sizeof(struct sockaddr_in);
	struct sockaddr_in serveraddr, clientaddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(argc != 3) {
		printUsage();
		return -1;
	}
	while((progargs = getopt(argc, argv, ":p:")) != -1) {
		switch(progargs) {
			case ':':
				printf("Missing argument.\n");
				printUsage();
				return -1;
				break;
			case '?':
				printf("Invalid option!.\n");
				printUsage();
				return -1;
				break;
			case 'p':
				if(setPort(optarg, &serveraddr) == -1) {
					printf("Invalid Port-Range. Must be between 1 - 65535.\n");
					return -1;
				}
		}
	}
	
	serversock = socket(AF_INET, SOCK_DGRAM, 0);
	
	err = bind(serversock, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
	if(err < 0) {
		return -1;
	}
	
	char recbuff[4096] ;
	while(gContinue) {
		recvfrom(serversock, recbuff, sizeof(recbuff), 0, (struct sockaddr*) &clientaddr, &flen);
        if(*recbuff == 1) {
		    addClient(recbuff, clientaddr, serversock);
		}
        
        //printf("Buffer: %i\n", *recbuff);
	}
	cleanClients();
	
	return 0;
}


void broadcastUserMessage(char* name, int serversock) {
    int i = 0;
    uint16_t h_namesize = (uint16_t)strlen(name);
    uint16_t n_namesize = htons(h_namesize);
    char* message = malloc((3+h_namesize)*sizeof(char));
    struct client* current = firstclient;
    *message = 3;
    memcpy(message+1, &n_namesize, sizeof(uint16_t));
    memcpy(message+3, name, h_namesize);
    for(i = 0; i < clientcount; i++) {
    	sendto(serversock, message, 3+h_namesize, 0, (struct sockaddr *)&(current->clientaddr), sizeof(current->clientaddr));
        current = current->next;
    }
}


int addClient(char* buff, struct sockaddr_in clientaddr, int serversock) {
	if(*buff != 1) {
		return -1;
	}
	socklen_t flen = sizeof(struct sockaddr_in);
    struct sockaddr_in newaddr;
	newaddr.sin_family = AF_INET;
	newaddr.sin_port = htons(0);
	newaddr.sin_addr.s_addr = INADDR_ANY;
    
    int err = 0;
	uint16_t usernamelength = 0;
	memcpy(&usernamelength, buff+1, sizeof(uint16_t));
	usernamelength = ntohs(usernamelength);
	char* name = malloc(sizeof(char) * (usernamelength));
	memcpy(name, buff+3, usernamelength*sizeof(char));
	printf("NAME: %s\n", name);
	char message[4];
	*message = 2;
	
	if(checkUsername(name) < 0 ) {
		
		printf("Unallowed Username!\n");
		*(message+1) = 1;
	}
	else {
    	int newsock = socket(AF_INET, SOCK_DGRAM, 0);
        err = bind(newsock, (struct sockaddr *) &newaddr, sizeof(newaddr));
        getsockname(newsock, (struct sockaddr *) &newaddr, &flen);
        
		printf("Username: %s with Length: %i\n", name, usernamelength);
		if(clientcount == 0) {
			firstclient = malloc(sizeof(struct client));
			firstclient->clientaddr = clientaddr;
			firstclient->username = malloc((usernamelength+1) * sizeof(char));
			strcpy(firstclient->username, name);
            firstclient->clientsock = newsock;
            
			currentclient = firstclient;
			
			printf("Clientnr: %i Name: %s\n", clientcount + 1, currentclient->username);
			++clientcount;
		}
		else {
			struct client* new = malloc(sizeof(struct client));
			new->clientaddr = clientaddr;
			new->username = malloc(usernamelength * sizeof(char));
			strcpy(new->username, name);
			currentclient->next = new;
            currentclient->clientsock = newsock;
			currentclient = new;
			printf("Clientnr: %i Name: %s\n", clientcount + 1, currentclient->username);

			++clientcount;
		}
		
		*(message+1) = 0;
		uint16_t port = currentclient->clientaddr.sin_port;
		memcpy(message+2, &port, sizeof(uint16_t));
	}
	sendto(serversock, message, 4, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
    broadcastUserMessage(name, serversock);
	free(name);
    
	return 0;
}


int cleanClients() {
	int i = 0;
	struct client* current;
	struct client* next;
	current = firstclient;
	for(i = 0; i < clientcount; i++) {
		free(current->username);
		next = current->next;
		free(current);
		current = next;
	}
	return 0;
}


/**
 * 	checks if Username contains only legal characters
 * 
 * 
 * 
 */
int checkUsername(char* username) {
	int i, userlength;
	userlength = strlen(username);
	char legalchars[] = "abcdefghijklmnopqrstuvwxyz1234567890";
	i = strspn(username, legalchars);
	if(userlength != i) {
		return -1;
	}
	else if(clientcount > 0) {
		struct client* current = firstclient;
		int i = 0;
		for(i = 0; i < clientcount; i++) {
			if(strlen(current->username) == strlen(username)) {
				if(strcmp(current->username, username) == 0) {
					return -1;
				}
			}
			current = current->next;
		}
	}
	return 0;
	
}


int setPort(char* portadress, struct sockaddr_in* serveraddr) {
	long int port;
	char* point;
	port = strtol(portadress, &point, 10);
	if(port < 0 || port > 65535 || point == NULL || *point != 0)
		return -1;
	else {
		serveraddr->sin_port = htons( ( (uint16_t) port ) );
		return 0;
	}
}

void printUsage() {
	printf("Usage: ./udp_chat_server -p <serv_port>\n");
}
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

/**
 *	handle SigInt for cleanup
 */
void handleSigInt(int param) {
	gContinue = 0;
}


int main(int argc, char** argv) {
	signal(SIGINT, handleSigInt);

	int err = 0;
	int progargs;
	int serversock;
	extern char *optarg;
	extern int optind, optopt;
	socklen_t flen = sizeof(struct sockaddr_in);
	struct sockaddr_in serveraddr, clientaddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//check input 
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
		
		struct timeval timeout = {0, 0};

		fd_set readfds;
		int err = 0;
	
		FD_ZERO(&readfds);	
	    FD_SET(serversock, &readfds);
        		
		err = select(serversock+1 , &readfds, NULL, NULL, &timeout);
        
		if(FD_ISSET(serversock, &readfds)) {
			recvfrom(serversock, recbuff, sizeof(recbuff), 0, (struct sockaddr*) &clientaddr, &flen);
			if(*recbuff == 1) {
				addClient(recbuff, clientaddr, serversock);
			}
		}
	}
	printf("shutting down server...\n");
	close(serversock);
	
	while(clientcount > 0) {
		pthread_join(firstclient->thread, NULL);
	}
	
	//cleanClients();
	
	pthread_exit(NULL);

	return 0;
}


void broadcastUserMessage(char* name) {
    int i = 0;
    uint16_t h_namesize = (uint16_t)strlen(name);
    uint16_t n_namesize = htons(h_namesize);
    char* message = malloc((3+h_namesize)*sizeof(char));
    struct client* current = firstclient;
    *message = 3;
    memcpy(message+1, &n_namesize, sizeof(uint16_t));
    memcpy(message+3, name, h_namesize);
    for(i = 0; i < clientcount; i++) {
    	sendto(current->clientsock, message, 3+h_namesize, 0, (struct sockaddr *)&(current->clientaddr), sizeof(current->clientaddr));
        current = current->next;        
    }
	free(message);
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
	char* name = malloc(sizeof(char) * (usernamelength+1));
	memcpy(name, buff+3, usernamelength*sizeof(char));
	name[usernamelength] = '\0';
	char message[4];
	*message = 2;
	
	if(checkUsername(name) < 0 ) {
		
		*(message+1) = 1;
		uint16_t initPort = 0;
		memcpy(message+2, &initPort, sizeof(uint16_t));
		sendto(serversock, message, 4, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
		free(name);
	}
	else {
    	int newsock = socket(AF_INET, SOCK_DGRAM, 0);
        err = bind(newsock, (struct sockaddr *) &newaddr, sizeof(newaddr));
        getsockname(newsock, (struct sockaddr *) &newaddr, &flen);
                
        
		if(clientcount == 0) {
			firstclient = malloc(sizeof(struct client));
			firstclient->clientaddr = clientaddr;
			firstclient->username =name;//malloc((usernamelength+2) * sizeof(char));
			//strcpy(firstclient->username, name);
            firstclient->clientsock = newsock;
            
			currentclient = firstclient;
		}
		else {
			struct client* new = malloc(sizeof(struct client));
			struct client* tmp = currentclient;
			new->clientaddr = clientaddr;
			new->username = name;//malloc((usernamelength+2) * sizeof(char));
			//strcpy(new->username, name);
            new->clientsock = newsock;
            currentclient->next = new;
			currentclient = new;
			currentclient->before = tmp;
		}
		++clientcount;
        *(message+1) = 0;
		uint16_t port = newaddr.sin_port;
		memcpy(message+2, &port, sizeof(uint16_t));
		sendto(serversock, message, 4, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
		broadcastUserMessage(name);
		int rc;
		rc = pthread_create(&(currentclient->thread), NULL, startUserThread, currentclient);
	}
	
	return 0;
}

void sendDisconnect(struct client* toDelete) {
	uint8_t disc = 7;
	sendto(toDelete->clientsock, &disc, 1, 0, (struct sockaddr*)&(toDelete->clientaddr), sizeof(toDelete->clientaddr));
}

void deleteClient(struct client* toDelete) {
	
	if(clientcount == 1) {
	}
	else if(toDelete == firstclient) {
		firstclient = toDelete->next;
		toDelete->next->before = firstclient->before;
	}
	else if(toDelete == currentclient) {
		toDelete->before->next = toDelete->next;
		currentclient = toDelete->before;
	}
	else {
		toDelete->before->next = toDelete->next;
		toDelete->next->before = toDelete->before;
	}
	--clientcount;

	if(gContinue) {
		sendDisconnect(toDelete);
		broadcastDisconnectMessage(toDelete->username);
	}
	close(toDelete->clientsock);
	free(toDelete->username);
	pthread_detach(toDelete->thread);
	free(toDelete);
}

void broadcastDisconnectMessage(char* username) {
	uint16_t h_usernamelength = strlen(username);
	uint16_t n_usernamelength = htons(h_usernamelength);
	
	char* message = malloc((4+h_usernamelength)*sizeof(char));
	*message = 8;
	memcpy(message+1,&n_usernamelength, sizeof(uint16_t));
	memcpy(message+3, username, h_usernamelength);
	
	struct client* current = firstclient;
	int i = 0;
    for(i = 0; i < clientcount; i++) {
    	sendto(current->clientsock, message, 3+h_usernamelength, 0, (struct sockaddr *)&(current->clientaddr), sizeof(current->clientaddr));
        current = current->next;        
    }
	free(message);
	
	
}

void *startUserThread(void* args) {
	struct client* cl = (struct client*) args;
	socklen_t flen = sizeof(struct sockaddr_in);
	int lostping = 0;
	
	struct timeval timeout = {5, 0};	
	
	char recbuff[1024];
	int tContinue = 1;
	fd_set readfds;
	int err = 0;
	

	while(tContinue && gContinue) {
		FD_ZERO(&readfds);	
	    FD_SET(cl->clientsock, &readfds);
	    FD_SET(0, &readfds);
        
		
		err = select(cl->clientsock+1 , &readfds, NULL, NULL, &timeout);
        
		if(FD_ISSET(cl->clientsock, &readfds)) {
			err = recvfrom(cl->clientsock, recbuff, sizeof(recbuff),0,(struct sockaddr*) &cl->clientaddr, &flen);
			if(*recbuff == 10) {
				lostping = 0;
			}
			if(*recbuff == 4) {
				broadcastMessage(recbuff, cl->username);
			}
			if(*recbuff == 6) {
				tContinue = 0;
			}
		}
		else {
			lostping++;
			if(lostping >= 4) {
				tContinue = 0;
			}
			else {
				sendPing(cl->clientsock, cl->clientaddr);
				timeout.tv_sec = 5;
				timeout.tv_usec = 0;
			}
		}
	}
	deleteClient(cl);
	pthread_exit(NULL);
}

void broadcastMessage(char* buff, char* username) {
	int i = 0;
    uint16_t h_namesize = (uint16_t)strlen(username);
    uint16_t n_namesize = htons(h_namesize);
	
	uint32_t h_messagelength;
	uint32_t n_messagelength;
	
	memcpy(&n_messagelength, buff+1, sizeof(uint32_t));
	h_messagelength = ntohl(n_messagelength);
	
	
    char* message = malloc((9+h_namesize+h_messagelength)*sizeof(char));
	*message = 5;
	memcpy(message+1, &n_namesize, sizeof(uint16_t));
	memcpy(message+3, username, h_namesize*sizeof(char));
	memcpy(message+3+h_namesize, &n_messagelength, sizeof(uint32_t));
	memcpy(message+7+h_namesize, buff+5, h_messagelength*sizeof(char));
		
    struct client* current = firstclient;
    for(i = 0; i < clientcount; i++) {
    	sendto(current->clientsock, message, 7+h_namesize+h_messagelength, 0, (struct sockaddr *)&(current->clientaddr), sizeof(current->clientaddr));
        current = current->next;        
    }
	free(message);
}

int cleanClients() {
	//int i = 0;
	//struct client* current;
	//struct client* next;
	//current = firstclient;
	while(clientcount > 0) {
		deleteClient(firstclient);
	}
	/*for(i = 0; i < clientcount; i++) {
		free(current->username);
		close(current->clientsock);
		next = current->next;
		free(current);
		current = next;
	}*/
	return 0;
}

/**
 *	send Ping to client
 */
int sendPing(int clientsock, struct sockaddr_in clientaddr) {
	int err = 0;
	uint8_t ping = 9;
	err = sendto(clientsock, &ping, 1, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
	if(err < 0)
		return -1;
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
			if(strlen(current->username) == userlength) {
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
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
	//signal handling
	signal(SIGINT, handleSigInt);

	int err = 0;
	int progargs;
	int serversock;
	extern char *optarg;
	extern int optind, optopt;
	socklen_t flen = sizeof(struct sockaddr_in);
	struct sockaddr_in serveraddr, clientaddr;
	
	//init server address
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
	//create socket for server requests
	serversock = socket(AF_INET, SOCK_DGRAM, 0);
	
	err = bind(serversock, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
	if(err < 0) {
		return -1;
	}
	
	//buffer for serverrequests
	char recbuff[4096];
	fd_set readfds;
	struct timeval timeout = {0, 0};

	while(gContinue > 0) {
		
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		FD_ZERO(&readfds);	
	    FD_SET(serversock, &readfds);
		
		//wait for messages at server socket
		err = select(serversock+1 , &readfds, NULL, NULL, &timeout);
		if(FD_ISSET(serversock, &readfds) && gContinue) {
			recvfrom(serversock, recbuff, sizeof(recbuff), 0, (struct sockaddr*) &clientaddr, &flen);
			
			//check if it is a connection request
			if(*recbuff == 1) {
				addClient(recbuff, clientaddr, serversock);
			}
		}
	}
	printf("\nshutting down server...\n");
	close(serversock);
	
	while(clientcount > 0) {
		pthread_join(firstclient->thread, NULL);
	}
		
	pthread_exit(NULL);

	return 0;
}

/**
 *	send message for new user to all users
 */
void broadcastUserMessage(char* name) {
    int i = 0;
	int err = 0;
	
    uint16_t h_namesize = (uint16_t)strlen(name);
    uint16_t n_namesize = htons(h_namesize);
	
    char* message = malloc((3+h_namesize)*sizeof(char));
	
    struct client* current = firstclient;
	
	//copy message into buffer
    *message = 3;
    memcpy(message+1, &n_namesize, sizeof(uint16_t));
    memcpy(message+3, name, h_namesize);
	
	//send message to every client
    for(i = 0; i < clientcount; i++) {
    	err = sendto(current->clientsock, message, 3+h_namesize, 0, (struct sockaddr *)&(current->clientaddr), sizeof(current->clientaddr));
		if(err < 0) {
			printf("Failed to send User Message to %s!\n", current->username);
		}
        current = current->next;        
    }
	free(message);
}

/**
 *	add new client to chat server
 */
int addClient(char* buff, struct sockaddr_in clientaddr, int serversock) {
	//check if buffer is correct
	if(*buff != 1) {
		return -1;
	}
	    
    int err = 0;
	socklen_t flen = sizeof(struct sockaddr_in);
	uint16_t h_usernamelength = 0;
	uint16_t n_usernamelength = 0;
	
    struct sockaddr_in newaddr;
	newaddr.sin_family = AF_INET;
	newaddr.sin_port = htons(0);
	newaddr.sin_addr.s_addr = INADDR_ANY;

	//copy length of username to malloc name buffer
	memcpy(&n_usernamelength, buff+1, sizeof(uint16_t));
	h_usernamelength = ntohs(n_usernamelength);
	
	char* name = malloc(sizeof(char) * (h_usernamelength+1));
	memcpy(name, buff+3, h_usernamelength*sizeof(char));
	name[h_usernamelength] = '\0';
	
	char message[4]; //buffer to answer the request
	*message = ID_SV_CON_REP; //set message id
	
	//check if username is valid
	if(checkUsername(name) < 0 ) {
		//if it is unvalid deny the request
		*(message+1) = 1;
		uint16_t initPort = 0;
		memcpy(message+2, &initPort, sizeof(uint16_t));
		sendto(serversock, message, 4, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
		free(name);
	}
	else {
		//else add client to linked list
    	int newsock = socket(AF_INET, SOCK_DGRAM, 0);
        err = bind(newsock, (struct sockaddr *) &newaddr, sizeof(newaddr));
        getsockname(newsock, (struct sockaddr *) &newaddr, &flen);
                
        
		if(clientcount == 0) {
			firstclient = malloc(sizeof(struct client));
			firstclient->clientaddr = clientaddr;
			firstclient->username =name;
            firstclient->clientsock = newsock;
            
			currentclient = firstclient;
		}
		else {
			struct client* new = malloc(sizeof(struct client));
			struct client* tmp = currentclient;
			new->clientaddr = clientaddr;
			new->username = name;
            new->clientsock = newsock;
            currentclient->next = new;
			currentclient = new;
			currentclient->before = tmp;
		}
		
		++clientcount;
		//answer the client with new port
        *(message+1) = 0; 
		uint16_t port = newaddr.sin_port;
		memcpy(message+2, &port, sizeof(uint16_t));
		sendto(serversock, message, 4, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
		broadcastUserMessage(name);
		//create new thread for client
		int rc;
		rc = pthread_create(&(currentclient->thread), NULL, startUserThread, currentclient);
	}
	
	return 0;
}

/**
 *	answer to disconnect request
 */
void sendDisconnect(struct client* toDelete) {
	uint8_t disc = 7;
	int err = 0;
	err = sendto(toDelete->clientsock, &disc, 1, 0, (struct sockaddr*)&(toDelete->clientaddr), sizeof(toDelete->clientaddr));
	if(err < 0) {
		printf("Sending disconnect to %s failed!\n", toDelete->username);
	}
}

/**
 *	delete a client
 */
void deleteClient(struct client* toDelete) {
	
	//if only 1 client only free and close
	if(clientcount == 1) {
	}
	//else rearrange the linked list
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
	
	//if client disconnects not during a server shutdown send disconnect message to all clients
	if(gContinue) {
		sendDisconnect(toDelete);
		broadcastDisconnectMessage(toDelete->username);
	}
	//free and close everything
	close(toDelete->clientsock);
	free(toDelete->username);
	pthread_detach(toDelete->thread);
	free(toDelete);
}

/**
 *	send message of disconnect to all clients
 */
void broadcastDisconnectMessage(char* username) {
	uint16_t h_usernamelength = strlen(username);
	uint16_t n_usernamelength = htons(h_usernamelength);
	
	//make message
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

/**
 *	user thread for pinging and message receive
 */
void *startUserThread(void* args) {
	struct client* cl = (struct client*) args; //pointer to client struct
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
        
		err = select(cl->clientsock+1 , &readfds, NULL, NULL, &timeout);
        
		if(FD_ISSET(cl->clientsock, &readfds)) {
			err = recvfrom(cl->clientsock, recbuff, sizeof(recbuff),0,(struct sockaddr*) &cl->clientaddr, &flen);
			//ping answer
			if(*recbuff == ID_CL_PING_REP) {
				lostping = 0;
			}
			//new message
			else if(*recbuff == ID_CL_MSG) {
				broadcastMessage(recbuff, cl->username);
			}
			//disconnect request
			else if(*recbuff == ID_CL_DISC_REQ) {
				tContinue = 0;
			}
		}
		//no answer to ping
		else {
			lostping++;
			//if 3 unanswered ping kill thread and client
			if(lostping >= 4) {
				tContinue = 0;
			}
			//else send new ping request and reset timeout
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

/**
 *	send message to all clients
 */
void broadcastMessage(char* buff, char* username) {
	int i = 0;
	int err = 0;
    uint16_t h_namesize = (uint16_t)strlen(username);
    uint16_t n_namesize = htons(h_namesize);
	
	uint32_t h_messagelength;
	uint32_t n_messagelength;
	
	memcpy(&n_messagelength, buff+1, sizeof(uint32_t));
	h_messagelength = ntohl(n_messagelength);
	
	//alloc message
    char* message = malloc((9+h_namesize+h_messagelength)*sizeof(char));
	
	//copy message
	*message = ID_SV_AMSG;
	memcpy(message+1, &n_namesize, sizeof(uint16_t));
	memcpy(message+3, username, h_namesize*sizeof(char));
	memcpy(message+3+h_namesize, &n_messagelength, sizeof(uint32_t));
	memcpy(message+7+h_namesize, buff+5, h_messagelength*sizeof(char));
	
	//send to all clients
    struct client* current = firstclient;
    for(i = 0; i < clientcount; i++) {
    	err = sendto(current->clientsock, message, 7+h_namesize+h_messagelength, 0, (struct sockaddr *)&(current->clientaddr), sizeof(current->clientaddr));
		if(err < 0) {
			printf("Sending message to %s failed!\n", current->username);
		}
        current = current->next;        
    }
	free(message);
}

/**
 *	send Ping to client
 */
int sendPing(int clientsock, struct sockaddr_in clientaddr) {
	int err = 0;
	uint8_t ping = ID_SV_PING_REQ;
	err = sendto(clientsock, &ping, 1, 0, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
	if(err < 0)
		return -1;
	return 0;
}

/**
 * 	checks if Username contains only legal characters
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

/**
 *	check if port is legal and set it in sockaddr
 */
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

/**
 *	print the usage
 */
void printUsage() {
	printf("Usage: ./udp_chat_server -p <serv_port>\n");
}
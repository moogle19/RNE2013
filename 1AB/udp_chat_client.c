#include "udp_chat_client.h"

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

void handleSigInt(int param) {
    gContinue = 0;
}

int main(int argc, char** argv) {
	int progargs;
	uint16_t messageport, userlength;
	//struct sockaddr_in messageaddr;
	extern char *optarg;
	char* ip = "";
	char* namepoint = "";
	extern int optind, optopt;
	
	struct sockaddr_in messageaddr;
	int err;
	int clientsock;
	
	int portnumber = -1;
	if(argc != 7) {
		printUsage();
		return -1;
	}
	while((progargs = getopt(argc, argv, "s:p:u:")) != -1) {
		switch (progargs) {
			case ':':
				printf("Missing arguments");
				return -1;
				break;
			case '?':
				printf("Invalid option!");
				return -1;
				break;
			case 's':
                ip = malloc((strlen(optarg)+1)*sizeof(char));
				strcpy(ip, optarg);
				if(checkip(optarg) == -1) {
					printf("IP-Adress invalid!\n"); return -1;}
				break;
			case 'p':
				portnumber = checkport(optarg);
				if(portnumber == -1) {
					printf("Portnumber invalid\n"); return -1;}
				break;
			case 'u':
				userlength = strlen(optarg);
				namepoint = malloc((userlength+1)*sizeof(char));
				if(checkusername(optarg, namepoint) == -1) {
					printf("Username invalid\n"); return -1;}
				break;
		}
	}
	
	clientsock = getClientsock();
	
	//connect to the server
    messageport = connectToSock(ip, portnumber, namepoint, clientsock);
    
    if(messageport == 0) {
		return -1;
	}
    
    messageaddr.sin_family = AF_INET;
	messageaddr.sin_port = htons(messageport);
	inet_pton(AF_INET, ip, &messageaddr.sin_addr);
	socklen_t flen = sizeof(struct sockaddr_in);
	uint8_t recbuff[1024];
	char consolebuffer[1024];
	
	recvfrom(clientsock, recbuff, sizeof(recbuff), 0, (struct sockaddr*) &messageaddr, &flen);
    
	getUserMessage(recbuff);
	 
    printRecBuffer(recbuff, clientsock, messageaddr);
	//int stdinfd = fcntl(STDIN_FILENO,  F_DUPFD, 0);
	fd_set readfds;

	FD_ZERO(&readfds);	
	FD_SET(clientsock, &readfds);
	FD_SET(0, &readfds);
	    
	while(1) {
	    FD_ZERO(&readfds);	
	    FD_SET(clientsock, &readfds);
	    FD_SET(0, &readfds);
        
		err = select(clientsock+1 , &readfds, NULL, NULL, NULL);
        
		if(FD_ISSET(clientsock, &readfds)) {
			err = recvfrom(clientsock, recbuff, sizeof(recbuff),0,(struct sockaddr*) &messageaddr, &flen); 
			printRecBuffer(recbuff, clientsock, messageaddr);
		}
		if(FD_ISSET(0, &readfds)) {
			 int consolelength = readline(0, consolebuffer, 1024);
			//if(consolelength > 0) {
				sendMessage(consolebuffer, clientsock, messageaddr);
                //}

		}
		
	   
		//sendMessage("test", clientsock, messageaddr);

		//err = recvfrom(clientsock, recbuff, sizeof(recbuff),0,(struct sockaddr*) &messageaddr, &flen);   
		//printRecBuffer(recbuff);
	}
    //getMessage(recbuff);
    printf("ID: %i\n", *recbuff);
    //cleanupr
    free(ip);
    free(namepoint);
    close(clientsock);
    
    while(gContinue) {
        
    }
	return 1;
}

void printUsage() {
	printf("%s\n", "Usage: ./udp_chat_client -s \"IP-Adress\" -p \"portnumber\" -u \"username\"");
}

void printRecBuffer(uint8_t* buff, int clientsocket, struct sockaddr_in messageaddr) {    
    switch(*buff) {
        /*case 2:		//getConnectionRequest(buff);
							break;*/
        case 3:     	getUserMessage(buff);
							break;
		case 5:	        getMessage(buff);
							break;
        case 9:         answerPing(clientsocket, messageaddr);
                            break;
        case 11:    	getServerMessage(buff);
							break;
        default:    	printf("Buffer: %s\n", buff+5);
							break;
    }
}

void answerPing(int clientsocket, struct sockaddr_in messageaddr) {
    uint8_t ping[1];
    *ping = 10;
	int err = sendto(clientsocket, ping, 1, 0, (struct sockaddr*) &messageaddr, sizeof(struct sockaddr_in));
	if(err < 0) {
		printf("Sending of Message failed!\n");
	}
}

void getUserMessage(uint8_t* buff) {
	if(*buff == 3) {
		uint16_t userlength = 0;
		memcpy(&userlength, (buff+1), sizeof(uint16_t));
        userlength = ntohs(userlength);
		char* retchar = malloc((userlength+1)*sizeof(char));
        retchar[userlength] = '\0';
		memcpy(retchar, (buff+3), (sizeof(char)*(userlength)));
		printf("%s hat den Chat betreten.\n", retchar);
		free(retchar);
	}
    else
        printf("Fail!\n");
}

void getServerMessage(uint8_t* message) {
    uint32_t messagelength = 0;
    int offset = 1;
    memcpy(&messagelength, (message+offset), sizeof(uint32_t));
    messagelength = ntohl(messagelength);
    offset += sizeof(uint32_t);
    char* out = malloc((messagelength+1)*sizeof(char));
    out[messagelength] = '\0';
    memcpy(out, message+offset, messagelength*sizeof(char));
    printf("#server#: %s\n", out);
    free(out);
}

void sendMessage(char* message, int clientsocket, struct sockaddr_in messageaddr) {
	uint32_t messagelength = strlen(message) - 1;
    char* discon = "/disconnect";
    if(messagelength == 11 && strncmp(message, discon, 11) == 0) {
        printf("Disconnect!\n");
    }
    else {
        uint32_t networklength = htonl(messagelength);
        uint8_t* sendbuff = malloc((5+messagelength)*sizeof(uint8_t));
        *sendbuff = 4;
        int offset = 1;
    	memcpy((sendbuff+offset), &networklength, sizeof(uint32_t));
        offset += sizeof(uint32_t);
    	memcpy(sendbuff+offset, message, messagelength*sizeof(char));

    	int err = sendto(clientsocket, sendbuff, (messagelength+5), 0, (struct sockaddr*) &messageaddr, sizeof(struct sockaddr_in));
    	if(err < 0) {
    		printf("Sending of Message failed!");
    	}
    }
    //free(sendbuff);
    
}


void getMessage(uint8_t* message) {
	uint16_t usernamelength = 0;
	uint32_t messagelength = 0;
	uint8_t offset = 1;
	char* username;
	char* usermessage;
    	
	memcpy(&usernamelength, message+offset, sizeof(uint16_t));
    usernamelength = ntohs(usernamelength);
	offset += sizeof(uint16_t);
	username = malloc((usernamelength+1) * sizeof(char));
	
	memcpy(username, message+offset, usernamelength*sizeof(char));
	offset += usernamelength;
	memcpy(&messagelength, (message+offset), sizeof(uint32_t));
    messagelength = ntohl(messagelength);
	offset += sizeof(uint32_t);
	usermessage = malloc((messagelength + 1) * sizeof(char));
	memcpy(usermessage, (message+offset),messagelength*sizeof(char));
	
	printf("<%s>: %s\n", username, usermessage);
	free(username);
	free(usermessage);
}

/**
 * 	generates Socket for client with own IP and a random port
 * 
 * 
 * 
 */
int getClientsock() {
	int err;
	struct sockaddr_in origin;
	//get Socket for connection
	int clientsock = socket(AF_INET, SOCK_DGRAM, 0);
	
	origin.sin_family = AF_INET;
	origin.sin_port = htons(4711);
	origin.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind our address to socket
	err = bind(clientsock, (struct sockaddr *) &origin, sizeof(origin));  
	//error checking
	if(err < 0) 
		return -1;
	return clientsock;
}

/**
 * 	checks if Username contains only legal characters
 * 
 * 
 * 
 */
int checkusername(char* username, char* namepoint) {
	int i, userlength;
	userlength = strlen(username);
	char legalchars[] = "abcdefghijklmnopqrstuvwxyz1234567890";
	i = strspn(username, legalchars);
	if(userlength != i)
		return -1;
	strcpy(namepoint, username);
	return 0;
	
}

/**
 * 	checks if port is in legal range
 * 
 * 
 * 
 */
int checkport(char* portadress) {
	long int port;
	char* point;
	port = strtol(portadress, &point, 10);
	if(port < 0 || port > 65535 || point == NULL || *point != 0)
		return -1;
	return (int)port;
}


/**
 * 	checks for legal ip adress
 * 
 * 
 * 
 */
int checkip(char* ipadress) {
	char* iptok;
	int blockcount = 0;
	char* point;
	long int tmp;
	iptok = strtok(ipadress, ".");
	while(iptok != NULL) {
		++blockcount;
		tmp = strtol(iptok, &point, 10);
		if((tmp < 0 || tmp > 255) || point == NULL || *point != 0)
			return -1;
		iptok = strtok(NULL, ".");
	}
	if(blockcount != 4)
		return -1;
	return 0;
}

/**
 *	Connects to server and returns port for messaging
 *
 *  @return socketnumber if successful, else -1
 */
int connectToSock(char* ipadress, int portnumber, char* name, int sock) { 
	
	int err = 0;
	struct sockaddr_in dest;
	
	uint16_t sendbuffersize = (3+strlen(name)); //1byte for id, 2byte for length + message
	uint8_t* sendbuffer = malloc(sendbuffersize*sizeof(uint8_t));
	
	uint8_t reqbuff[1024];
	fd_set readfds;
	
	struct timeval timeout = {5, 0};	
	
	//set socket address of the server
	dest.sin_family = AF_INET;
	dest.sin_port = htons(portnumber);
	//inet_aton(ipadress, &dest.sin_addr);
	inet_pton(AF_INET, ipadress, &dest.sin_addr);
	
	socklen_t flen = sizeof(struct sockaddr_in);
	
	//get Hostname
	char hostname[1025] = "testserver";
	hostname[1024] = '\0';
	//err = getnameinfo((struct sockaddr*) &dest, flen, hostname, 1024, NULL, 0, 0);
	if(err < 0) {
		fprintf(stderr, "hostname error");
	}
	
	
	FD_ZERO(&readfds);	
	FD_SET(sock, &readfds);
	
	uint16_t transLength = htons(strlen(name)); //anemlength to network format
	//set message id
	
	//fill sendbuffer
	setClconreq(sendbuffer, transLength, name);
	
	//setClconreq(sendbuffer, transLength, name);
	
	
	printf("Verbinde als %s zu Server %s (%s) auf Port %i.\n", name, hostname, ipadress, portnumber);
	
	int i = 0;
	for(i = 0; i < 3; i++) {
		timeout.tv_sec = 5;
		//send message
		err = sendto(sock, sendbuffer, sendbuffersize, 0, (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
		if(err < 0)  //error checking
			return printError();
		
		//wait for answer of server
		err = select(sock + 1, &readfds, NULL, NULL, &timeout);
		
		//if server answered receive message
		if(FD_ISSET(sock, &readfds)) {
			err = recvfrom(sock, reqbuff, sizeof(reqbuff),0,(struct sockaddr*) &dest, &flen);
			break;
		}
		
		if(i < 2)
			printf("No Answer. Sending new request!\n");
		else {
			printf("Verbindung fehlgeschlagen. Server antwortet nicht.\n");
			return -1;
		}
	}	

	if(err < 0)
		return printError();
	
	printf("%s", reqbuff+4);
	uint16_t portnew = checkClconreq(reqbuff);
		
	free(sendbuffer);
	//close(sock);
	return portnew;
}

void disconServer(struct sockaddr_in server, int sock) {
	int err = 0;
	
	uint8_t sendbuffer = 6;
	
	uint8_t reqbuff[1024];
	fd_set readfds;
	
	struct timeval timeout = {5, 0};	
	
	socklen_t flen = sizeof(struct sockaddr_in);
	
	FD_ZERO(&readfds);	
	FD_SET(sock, &readfds);
	
	
	
	printf("Verbinde als %s zu Server %s (%s) auf Port %i.\n", name, hostname, ipadress, portnumber);
	
	int i = 0;
	for(i = 0; i < 3; i++) {
		timeout.tv_sec = 5;
		//send message
		err = sendto(sock, &sendbuffer, 1, 0, (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
		if(err < 0)  //error checking
			return printError();
		
		//wait for answer of server
		err = select(sock + 1, &readfds, NULL, NULL, &timeout);
		
		//if server answered receive message
		if(FD_ISSET(sock, &readfds)) {
			err = recvfrom(sock, reqbuff, sizeof(reqbuff),0,(struct sockaddr*) &dest, &flen);
			break;
		}
		
		if(i > 2 && *reqbuff == 7) {
			printf("Verbindung fehlgeschlagen. Server antwortet nicht.\n");
			return -1;
		}
	}	

	if(err < 0)
		return printError();
	
	printf("Disconnect: %i\n", *reqbuff);
}

/**
 * 	set CL_CON_REQ Message
 * 
 * 
 */
void setClconreq(uint8_t* sendbuff, uint16_t length, char* name) {
	*sendbuff = ID_CL_CON_REQ; //message ID
	memcpy((sendbuff+1), &length, sizeof(uint16_t));
	memcpy((sendbuff+3), name, strlen(name)*sizeof(char));
}

/**
 * 	get Port from CL_CON_REQ Answer
 * 
 * 
 * 
 */
uint16_t checkClconreq(uint8_t* buff) {
	uint16_t retport = 0;
	if(*buff == 2) {
		if(*(buff+1) == 0) {
			memcpy(&retport, (buff+2), sizeof(uint16_t));
			retport = ntohs(retport);
			printf("Verbindung akzeptiert. Der Port für die weitere Kommunikation lautet %i.\n", retport);
			return retport;
		}
		else {
			printf("Verbindung abgelehnt.\n");
			return 0;
		}
	}
	else {
		printf("Something went wrong!\n");
		return 0;
	}
}


int printError() {
	printf("%s\n", "Error");
	return -1;
}


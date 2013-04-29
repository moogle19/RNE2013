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
	
	char sendbuff[8];
	sendbuff[0] = 4;
	sendbuff[1] = 0;
	sendbuff[2] = 0;
	sendbuff[3] = 0;
	sendbuff[4] = 3;
	sendbuff[5] = 'h';
	sendbuff[6] = 'i';
	sendbuff[7] = '\0';
	
		
	//err = recvfrom(clientsock, recbuff, sizeof(recbuff),0,(struct sockaddr*) &messageaddr, &flen);
	
	//printf("\n Message: %s", recbuff);	
	
	err = sendto(clientsock, sendbuff, sizeof(sendbuff), 0, (struct sockaddr*) &messageaddr, sizeof(struct sockaddr_in));
	if(err < 0) {
		printf("sendto fail");
	}
	err = recvfrom(clientsock, recbuff, sizeof(recbuff),0,(struct sockaddr*) &messageaddr, &flen);
	
	printf("\n Message: %s", recbuff);
    

    //cleanup
    free(ip);
    free(namepoint);
    close(clientsock);

	return 1;
}


void printUsage() {
	printf("%s\n", "Usage: ./udp_chat_client -s \"IP-Adress\" -p \"portnumber\" -u \"username\"");
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


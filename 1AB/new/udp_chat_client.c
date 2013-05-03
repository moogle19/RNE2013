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
	uint16_t userlength;
	//struct sockaddr_in messageaddr;
	extern char *optarg;
	char username[65535];
	socklen_t flen = sizeof(struct sockaddr_in);
	extern int optind, optopt;
	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	
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
				printUsage();
				return -1;
				break;
			case 's':
				if(checkIp(optarg, &serveraddr) == -1) {
					printf("IP-Adress invalid!\n"); 
					return -1;
					}
				break;
			case 'p':
				portnumber = checkPort(optarg, &serveraddr);
				if(portnumber == -1) {
					printf("Portnumber invalid\n"); return -1;}
				break;
			case 'u':
				userlength = strlen(optarg);
				if(userlength > 65535) {
					printf("Username too long!\n");
					return -1;
				}
				if(checkUsername(optarg, username) == -1) {
					printf("Username invalid\n"); return -1;}
				break;
		}
	}
	
	//get Socket
	clientsock = getClientsock();
	
	//connect to the Messageserver
	connectToServer(&serveraddr, username, clientsock);
	
	uint8_t recbuff[4096];
	char consolebuffer[4096];
	
	fd_set readfds;
	int err = 0;
	while(gContinue) {
	    FD_ZERO(&readfds);	
	    FD_SET(clientsock, &readfds);
	    FD_SET(0, &readfds);
        
		err = select(clientsock+1 , &readfds, NULL, NULL, NULL);
        
		if(FD_ISSET(clientsock, &readfds)) {
			err = recvfrom(clientsock, recbuff, sizeof(recbuff),0,(struct sockaddr*) &serveraddr, &flen); 
			parseRecBuffer(recbuff, clientsock, serveraddr);
		}
		if(FD_ISSET(0, &readfds)) {
			int consolelength = readline(0, consolebuffer, 1024);
			if(consolelength > 0) {
				sendMessage(consolebuffer, serveraddr, clientsock);
             }

		}
	}
	
	
	return 1;
}

int disconnectFromServer(struct sockaddr_in* serveraddr, int clientsock) {
	int err = 0;
	
	uint8_t reqbuff[1024];
	fd_set readfds;
	
	struct timeval timeout = {5, 0};	
	
	socklen_t flen = sizeof(struct sockaddr_in);
	
	//get Hostname
	char hostname[1025] = "testserver";
	hostname[1024] = '\0';
	//err = getnameinfo((struct sockaddr*) &dest, flen, hostname, 1024, NULL, 0, 0);
	if(err < 0) {
		fprintf(stderr, "hostname error");
	}
	
	FD_ZERO(&readfds);	
	FD_SET(clientsock, &readfds);
		
	char ipstring[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(serveraddr->sin_addr), ipstring, INET_ADDRSTRLEN);
	
    printf("Beende die Verbindung zu Server %s (%s).\n", hostname, ipstring);
    
        
	int i = 0;
	for(i = 0; i < 3; i++) {
		timeout.tv_sec = 5;
		//send message
		if(sendDisconnect(serveraddr, clientsock) < 0) {
			printf("Sending Disc-Request failed!\n");
		}
		
		
		//wait for answer of server
		err = select(clientsock + 1, &readfds, NULL, NULL, &timeout);
		
		//if server answered receive message
		if(FD_ISSET(clientsock, &readfds)) {
			err = recvfrom(clientsock, reqbuff, sizeof(reqbuff),0,(struct sockaddr*) serveraddr, &flen);
			break;
		}
		
		if(i < 2)
			printf("No Answer. Sending new request!\n");
		else {
			printf("Verbindung nicht erfolgreich beendet. Timeout.\n");
            gContinue = 0;
			return -1;
		}
	}

	if(err < 0)
		return -1;
    
    if(*reqbuff == 7) {
        gContinue = 0;
        printf("Verbindung erfolgreich beendet.\n");
        return 0;
    }
    else {
        gContinue = 0;
        printf("Falsche Nachricht vom Server erhalten.\n");
        return -1;
    }
    
    
    
	return -1;
}

int sendDisconnect(struct sockaddr_in* serveraddr, int clientsock) {
	int err = 0;
	uint8_t id = 6;
	
   
	err = sendto(clientsock, &id, 1, 0, (struct sockaddr*) serveraddr, sizeof(struct sockaddr_in));
	if(err < 0)  //error checking
		return -1;
	return 0;
}


void sendMessage(char* message, struct sockaddr_in serveraddr,  int clientsock) {
	uint32_t messagelength = strlen(message) - 1;
    char* discon = "/disconnect";
    if(messagelength == 11 && strncmp(message, discon, 11) == 0) {
        printf("Disconnect!\n");
        disconnectFromServer(&serveraddr, clientsock);
    }
    else {
        uint32_t networklength = htonl(messagelength);
        uint8_t* sendbuff = malloc((5+messagelength)*sizeof(uint8_t));
        *sendbuff = 4;
        int offset = 1;
    	memcpy((sendbuff+offset), &networklength, sizeof(uint32_t));
        offset += sizeof(uint32_t);
    	memcpy(sendbuff+offset, message, messagelength*sizeof(char));

    	int err = sendto(clientsock, sendbuff, (messagelength+5), 0, (struct sockaddr*) &serveraddr, sizeof(struct sockaddr_in));
		free(sendbuff);
    	if(err < 0) {
    		printf("Sending of Message failed!");
    	}
    }
    
}


void parseRecBuffer(uint8_t* buff, int clientsocket, struct sockaddr_in serveraddr) {    
    switch(*buff) {
        case 3:     	printUserMessage(buff);
							break;
		case 5:	        printMessage(buff);
							break;
        case 8:         printDisconnectMessage(buff);
                        break;
        case 9:         sendPing(serveraddr, clientsocket);
                            break;
        case 11:    	printServerMessage(buff);
							break;
        default:    		printf("ID: %i Buffer: %s\n", *buff, buff+5);
							break;
    }
}

void printDisconnectMessage(uint8_t* buff) {
	if(*buff == 8) {
		uint16_t userlength = 0;
		memcpy(&userlength, (buff+1), sizeof(uint16_t));
        userlength = ntohs(userlength);
		char* retchar = malloc((userlength+1)*sizeof(char));
        retchar[userlength] = '\0';
		memcpy(retchar, (buff+3), (sizeof(char)*(userlength)));
		printf("%s hat den Chat verlassen.\n", retchar);
		free(retchar);
	}
    else
        printf("Fail!\n");
}

/**
 *	answer to a ping request
 *
 *
 */
void sendPing(struct sockaddr_in messageaddr, int clientsocket) {
    uint8_t ping[1];
    *ping = 10;
	int err = sendto(clientsocket, ping, 1, 0, (struct sockaddr*) &messageaddr, sizeof(struct sockaddr_in));
	if(err < 0) {
		printf("Sending of Message failed!\n");
	}
}


/**
 *	print messages
 *
 *
 */
void printMessage(uint8_t* message) {
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
	username[usernamelength] = '\0';
	offset += usernamelength;
	memcpy(&messagelength, (message+offset), sizeof(uint32_t));
    messagelength = ntohl(messagelength);
	offset += sizeof(uint32_t);
	usermessage = malloc((messagelength+1) * sizeof(char));
	memcpy(usermessage, (message+offset),messagelength*sizeof(char));
	usermessage[messagelength] = '\0';
	
	printf("<%s>: %s\n", username, usermessage);
	free(username);
	free(usermessage);
}

/**
 *	print messages from the server
 *
 *
 */
void printServerMessage(uint8_t* message) {
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

/**
 *	print messages for a new user
 *
 *
 */
void printUserMessage(uint8_t* buff) {
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

/**
 *	Send connectionrequest to server
 *
 *
 */
 int connectToServer(struct sockaddr_in* serveraddr, char* username, int clientsock) {
	int err = 0;
	
	uint8_t reqbuff[1024];
	fd_set readfds;
	
	struct timeval timeout = {5, 0};	
	
	socklen_t flen = sizeof(struct sockaddr_in);
	
	//get Hostname
	char hostname[1025] = "testserver";
	hostname[1024] = '\0';
	//err = getnameinfo((struct sockaddr*) &dest, flen, hostname, 1024, NULL, 0, 0);
	if(err < 0) {
		fprintf(stderr, "hostname error");
	}
	
	FD_ZERO(&readfds);	
	FD_SET(clientsock, &readfds);
		
	char ipstring[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(serveraddr->sin_addr), ipstring, INET_ADDRSTRLEN);
	
	printf("Verbinde als %s zu Server %s (%s) auf Port %i.\n", username, hostname, ipstring, ntohs(serveraddr->sin_port));
	
	int i = 0;
	for(i = 0; i < 3; i++) {
		timeout.tv_sec = 5;
		//send message
		if(sendConnectionRequest(clientsock, serveraddr, username) < 0) {
			printf("Sending failed!\n");
		}
		
		
		//wait for answer of server
		err = select(clientsock + 1, &readfds, NULL, NULL, &timeout);
		
		//if server answered receive message
		if(FD_ISSET(clientsock, &readfds)) {
			err = recvfrom(clientsock, reqbuff, sizeof(reqbuff),0,(struct sockaddr*) serveraddr, &flen);
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
		return -1;
	
	if(*reqbuff == 2) {
		if(*(reqbuff+1) == 0) {
			uint16_t serverport = 0;
			memcpy(&serverport, reqbuff+2, sizeof(uint16_t));
			serveraddr->sin_port = serverport;
			printf("Verbindung akzeptiert. Der Port für die weitere Kommunikation lautet %i.\n", ntohs(serveraddr->sin_port));
			return 0;
		}
		else {
			printf("Verbindung abgelehnt!\n");
			return -1;
		}
	}
	else {
		printf("Wrong answer from server!\n");
		return -1;
	}
	return -1;
 }
 
 /**
  *	 send connection request to server
  *
  *
  */
 int sendConnectionRequest(int clientsock, struct sockaddr_in* serveraddr, char* username) {
	int err = 0;
	uint16_t sendbuffersize = (3+strlen(username)); //1byte for id, 2byte for length + message
	uint8_t* sendbuffer = malloc(sendbuffersize*sizeof(uint8_t));
	uint16_t h_namelength = (uint16_t) strlen(username);
	uint16_t n_namelength = htons(h_namelength);
	int offset = 0;
	
	*sendbuffer = ID_CL_CON_REQ;
	++offset;
	
	memcpy(sendbuffer+offset, &n_namelength, sizeof(uint16_t));
	offset += sizeof(uint16_t);
	
	memcpy(sendbuffer+offset, username, h_namelength*sizeof(char));
	offset += h_namelength*sizeof(char);
	
	err = sendto(clientsock, sendbuffer, offset, 0, (struct sockaddr*) serveraddr, sizeof(struct sockaddr_in));
	free(sendbuffer);
	if(err < 0)  //error checking
		return -1;
	return 0;
 }

/**
 * 	checks for legal ip adress
 * 
 * 
 * 
 */
int checkIp(char* ipadress, struct sockaddr_in *serveraddr) {
	char* iptok;
	char ip[INET_ADDRSTRLEN];
	strcpy(ip, ipadress);
	int blockcount = 0;
	char* point;
	long int tmp;
	
	iptok = strtok(ipadress, "."); //divide string with '.'
	
	while(iptok != NULL) {
		++blockcount;
		tmp = strtol(iptok, &point, 10);
		if((tmp < 0 || tmp > 255) || point == NULL || *point != 0)
			return -1;
		iptok = strtok(NULL, ".");
	}
	if(blockcount != 4)
		return -1;
	else {
		inet_pton(AF_INET, ip, &(serveraddr->sin_addr));
		return 0;
	}
}

/**
 * 	checks if port is in legal range
 * 
 * 
 * 
 */
int checkPort(char* portadress, struct sockaddr_in *serveraddr) {
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
 * 	checks if Username contains only legal characters
 * 
 * 
 * 
 */
int checkUsername(char* username, char* namepoint) {
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
 *	Usage of the programm
 *
 *
 */
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
	origin.sin_port = htons(0);
	origin.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind our address to socket
	err = bind(clientsock, (struct sockaddr *) &origin, sizeof(origin));  
	//error checking
	if(err < 0) 
		return -1;
	return clientsock;
}
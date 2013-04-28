#include "udp_chat_client_connect.h"

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


/*
 *
 *
 *  @return socketnumber if successful, else -1
*/
int connectToSock(char* ipadress, int portnumber, char* name) { 
	
	int sock, err;
	struct sockaddr_in origin, dest;
	int sendbuffersize = (3+strlen(name));
	char* sendbuffer = malloc(sendbuffersize*sizeof(char));
	char receiverbuffer[32];
	fd_set readfds;
	struct timeval timeout = {5, 0};
	//create socket 
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	//set socket address of our client  
	origin.sin_family = AF_INET;
	origin.sin_port = htons(portnumber);
	origin.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//set socket address of the server
	dest.sin_family = AF_INET;
	dest.sin_port = htons(portnumber);
	inet_aton(ipadress, &dest.sin_addr);

	//bind our address to socket
	err = bind(sock, (struct sockaddr *) &origin, sizeof(origin));  
	//error checking
	if(err < 0) 
		return printError();
	
	socklen_t flen = sizeof(struct sockaddr_in);
	
	FD_ZERO(&readfds);	
	FD_SET(sock, &readfds);
	
	//set message id
	sendbuffer[0] = ID_CL_CON_REQ; //message ID
	sendbuffer[1] = 0; 
	sendbuffer[2] = strlen(name);
	
	int i = 0;
	for(i = 0; i < strlen(name) + 0; i++) {
		sendbuffer[i+3] = name[i];
	}
	
	printf("%s", sendbuffer);
	printf("connecting...\n");
	
	for(i = 0; i < 3; i++) {
		//send connectionrequest
		timeout.tv_sec = 5;
		//send message
		err = sendto(sock, sendbuffer, sendbuffersize, 0, (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
		if(err < 0)  //error checking
			return printError();
		
		//wait for answer of server
		err = select(sock + 1, &readfds, NULL, NULL, &timeout);
		
		//if server answered receive message
		if(FD_ISSET(sock, &readfds)) {
			err = recvfrom(sock, receiverbuffer, sizeof(receiverbuffer),0,(struct sockaddr*) &ipadress, &flen);
			break;
		}
		
		if(i < 2)
			printf("No Answer. Sending new request!\n");
		else {
			printf("Verbindung fehlgeschlagen. Server antwortet nicht.\n");
			return -1;
		}
	}
	
	//err = recvfrom(sock, sendbuffer, sizeof(sendbuffer),0,(struct sockaddr*) &ipadress, &flen);
	

	if(err < 0)
		return printError();
	
	uint16_t portnew = checkClconreq(receiverbuffer);
	//int i = 0;
	/*if(receiverbuffer[0] == 2) {
		if(receiverbuffer[1] == 0) {
			portnew = (uint16_t) receiverbuffer[2];
			portnew = portnew << 8;
			portnew += (uint16_t)receiverbuffer[3];
		}
	}
	portnew = ntohs(portnew);*/
	printf("Port is: %i\n", portnew);
	for(i = 0; i< 4; i++) {
		printf("%i\n", (uint8_t)receiverbuffer[i]);
	}
	printf("\n");
	//printf("%s\n", receiverbuffer);
	
	int newsock = socket(AF_INET, SOCK_DGRAM, 0);
	
	//set socket address of the server
	dest.sin_family = AF_INET;
	dest.sin_port = htons(portnew);
	inet_aton(ipadress, &dest.sin_addr);
	
	
	
	free(sendbuffer);
	return 0;
}

uint16_t checkClconreq(char* buff) {
	uint16_t retport = 0;
	if(buff[0] == 2) {
		if(buff[1] == 0) {
			retport = (uint16_t) buff[2];
			retport = retport << 8;
			retport += (uint16_t) buff[3];
			retport = ntohs(retport);
			return retport;
		}
		return 0;
	}
	else {
		return 0;
	}
}


int printError() {
	printf("%s\n", "Error");
	return -1;
}

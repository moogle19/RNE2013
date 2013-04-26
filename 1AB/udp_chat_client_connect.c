#include "udp_chat_client_connect.h"


/*
 *
 *
 *  @return socketnumber if successful, else -1
*/
int connectToSock(char* ipadress, int portnumber, char* name) {
    int sock, err;
    struct sockaddr_in origin, dest;
    uint8_t sendbuffer[4];
	char receiverbuffer[4096];
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
    if(err < 0)     //error checking 
        return printError();
    
    //copy connectionrequest to buffer
    
	sendbuffer[0] = 1;
	sendbuffer[1] = 0;
	sendbuffer[2] = 1;
	sendbuffer[3] = 0;

    socklen_t flen = sizeof(struct sockaddr_in);
    FD_ZERO(&readfds);
        
    FD_SET(sock, &readfds);  
	
	printf("connecting...\n");
    int i = 0;
    for(i = 0; i < 3; i++) {
        //send connectionrequest
		timeout.tv_sec = 5;
        err = sendto(sock, sendbuffer, sizeof(sendbuffer), 0, (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
        if(err < 0)     //error checking
            return printError();

        err = select(sock + 1, &readfds, NULL, NULL, &timeout);

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
    printf("%s\n", receiverbuffer);

	return 0;
}



int printError() {
    printf("%s\n", "Error");
    return -1;
}

#include "udp_chat_client.h"

int main(int argc, char** argv) {

	int progargs, sock;
    int err;
	extern char *optarg;
	char* ip;
    char buff[16];
	extern int optind, optopt;
	struct sockaddr_in adress, dest;
	int portnumber;
	if(argc != 7) {
		printUsage();
		return -1;
	}

	while((progargs = getopt(argc, argv, "s:p:u:")) != -1) {
		switch (progargs) {
			case ':':
				printf("Missing arguments");
				break;
			case 's':
                ip = malloc((strlen(optarg)+1)*sizeof(char));
				strcpy(ip, optarg);
				if(checkip(optarg) == -1)
					printf("IP-Adress invalid!\n");
				else
					printf("IP-Adress is good!\n");
			
				break;
			case 'p':
				portnumber = checkport(optarg);
				if(portnumber == -1)
					printf("Portnumber invalid\n");
				else
					printf("Portnumber is good\n");
				break;
			case 'u':
				if(checkusername(optarg) == -1)
					printf("Username invalid\n");
				else
					printf("Username is good\n");
				break;
		}
	}


	sock = socket(AF_INET, SOCK_DGRAM, 0);

	adress.sin_family = AF_INET;
	adress.sin_port = htons(portnumber);
    adress.sin_addr.s_addr = htonl(INADDR_ANY);
    
    dest.sin_family = AF_INET;
    dest.sin_port = htons(portnumber);
	inet_aton(ip, &dest.sin_addr);

	printf("%s\n", inet_ntoa(dest.sin_addr));
    
    err = bind(sock, (struct sockaddr *) &adress, sizeof(adress));	
    if(err < 0) 
        return printError();
    
    strcpy(buff, "SV_CON_REP 0x02");
    err = sendto(sock, buff, sizeof(buff), 0, (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
    if(err < 0)
        return printError();
    socklen_t flen = sizeof(struct sockaddr_in);

    err = recvfrom(sock, buff, sizeof(buff),0,(struct sockaddr*) &adress, &flen);
    if(err < 0)
        return printError();
    printf("%s", buff);
    

    //cleanup
    free(ip);
    close(sock);	

	return 1;
}

int printError() {
    printf("%s\n", "Error");
    return -1;
}

void printUsage() {
	printf("%s\n", "Usage: ./udp_chat_client -s \"IP-Adress\" -p \"portnumber\" -u \"username\"");
}

int checkusername(char* username) {
	int i, userlength;
	userlength = strlen(username);
	printf("%i\n", userlength);
	char legalchars[] = "abcdefghijklmnopqrstuvwxyz1234567890";
	i = strspn(username, legalchars);
	printf("%i\n", i);
	if(userlength != i)
		return -1;
	else
		return 0;
	
}

int checkport(char* portadress) {
	long int port;
	char* point;
	port = strtol(portadress, &point, 10);
	if(port < 0 || port > 65535 || point == NULL || *point != 0)
		return -1;
	return (int)port;
}

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

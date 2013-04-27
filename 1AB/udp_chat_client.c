#include "udp_chat_client.h"
#include "udp_chat_client_connect.h"

int main(int argc, char** argv) {

	int progargs, socket;
	extern char *optarg;
	char* ip = "";
	extern int optind, optopt;
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
				if(checkusername(optarg) == -1) {
					printf("Username invalid\n"); return -1;}
				break;
		}
	}

    socket = connectToSock(ip, portnumber, "test");
    
	
    

    //cleanup
    free(ip);
    close(socket);	

	return 1;
}


void printUsage() {
	printf("%s\n", "Usage: ./udp_chat_client -s \"IP-Adress\" -p \"portnumber\" -u \"username\"");
}

int checkusername(char* username) {
	int i, userlength;
	userlength = strlen(username);
	char legalchars[] = "abcdefghijklmnopqrstuvwxyz1234567890";
	i = strspn(username, legalchars);
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

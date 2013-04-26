#include "udp_chat_client.h"

int main(int argc, char** argv) {

int progargs;
char* arguments;
extern char *optarg;
extern int optind, optopt;

if(argc != 7) {
	printUsage();
	return -1;
}

printf("%s\n", argv[6]);

while((progargs = getopt(argc, argv, "s:p:u:")) != -1) {
	switch (progargs) {
		case ':':
			printf("Missing arguments");
			break;
		case 's':
			if(checkip(optarg) == -1)
				printf("IP-Adress invalid!\n");
			else
				printf("IP-Adress is good!\n");
			
			break;
		case 'p':
			if(checkport(optarg) == -1)
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




return 1;
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
	return 0;
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

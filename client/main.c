#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUF_SIZE 4096

int main(int argc, char** argv) {
	char* host;
	int portNum;
	int udpsock;
	int tcpsock;	
	struct hostent* hp;
	struct sockaddr_in sin;
	struct addrinfo hints, *serv_addr;

	if(argc != 3) {
		perror("Invalid number of arguments");
		exit(1);
	} else {
		host = argv[1];
		portNum = atoi(argv[2]);	
	}
	
	hp = gethostbyname(host);
	if(!hp) {
		perror("Unknown host");
		exit(1);
	}

	// build address data structure for udp
	bzero((char*)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char*)&sin.sin_addr, hp->h_length);
	sin.sin_port = htons(portNum);

	if((udpsock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Socket creation failed");
		exit(1);
	}
 
	bzero((char*)&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if(getaddrinfo(host, portNum, &hints, &serv_addr) != 0) {
		perror("Failed to resolve server");
		exit(1);
	}

	// open tcp socket
	if((tcpsock = socket(serv_addr->ai_family, serv_addr->ai_socktype, serv_addr->ai_protocol)) < 0) {
		perror("Failed to open socket");
	}
	if(connect(tcpsock, serv_addr->ai_addr, serv_addr->ai_addrlen) != 0) {
		perror("Failed to connect to server");
	}
	
}

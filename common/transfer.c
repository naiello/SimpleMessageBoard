#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "transfer.h"

ssize_t send_file(int sockfd, const char *filename)
{
	FILE *file = fopen(filename, "r");
	ssize_t filesz, sent = 0, net_filesz, current;
	char buffer[XFER_BUFSZ];

	if (!file) {
		return -2;
	}

	// get file size
	fseek(file, 0, SEEK_END);
	filesz = ftell(file);
	net_filesz = htonl(filesz);
	rewind(file);

	if (send(sockfd, &net_filesz, sizeof(net_filesz), 0) < 0) {
		return -1;
	}

	// send file in chunks		
	while (sent < filesz) {
		current = fread(buffer, sizeof(char), sizeof(buffer), file);
		if (send(sockfd, buffer, current, 0) < current) {
			return sent;
		}
		sent += current;
	}

	return sent;
}

ssize_t recv_file(int sockfd, const char *filename) 
{
	FILE *file;
	size_t recvd = 0, current, filesz;
	char buffer[XFER_BUFSZ];

	// check to see if file already exists
	if (access(filename, F_OK) == 0) {
		return -3;
	}

	file = fopen(filename, "w");
	if (!file) {
		return -2;
	}

	// receive the file size
	if (recv(sockfd, &filesz, sizeof(filesz), 0) < 0) {
		return -1;
	}
	filesz = ntohl(filesz);

	while (recvd < filesz) { 
		if ((current = recv(sockfd, buffer, sizeof(buffer), 0)) < 0) {
			return recvd;
		}

		fwrite(buffer, sizeof(char), current, file);
		recvd += current;
	}

	return recvd;
}

#include <error.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../common/cmddef.h"
#include "userhash.h"

#define BACKLOG 10

int main(int argc, char **argv) 
{
	char *port, *adminpass;
	struct addrinfo hints, *dg_server, *server;
	struct sockaddr_in *client_addr, *dg_client_addr;
	int sockfd, destfd, dg_sockfd;
	int yes = 1;
	uint16_t cmd;
	ssize_t xfer_sz;
	userhash users;
	socklen_t addrlen;
	int done = 0;
	char user[20], pass[20];
	uint16_t size;
	userinfo *uinfo = NULL;

	if (argc != 3) {
		error(1, 0, "Usage: %s [port] [admin-pass]\n", argv[0]);
	}

	port = argv[1];
	adminpass = argv[2];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// resolve TCP addr
	if (getaddrinfo(NULL, port, &hints, &server)) {
		error(1, 0, "Failed to get addr info.\n");
	}

	// resolve UDP addr
	hints.ai_socktype = SOCK_DGRAM;
	if (getaddrinfo(NULL, port, &hints, &dg_server)) {
		error(1, 0, "Failed to get UDP addr info.\n");
	}

	// open TCP socket
	if ((sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol)) == -1) {
		error(1, errno, "Failed to open TCP socket.\n");
	}

	// open UDP socket
	if ((dg_sockfd = socket(dg_server->ai_family, dg_server->ai_socktype, dg_server->ai_protocol)) == -1) {
		error(1, errno, "Failed to open UDP socket.\n");
	}

	// allow sockets to be reused
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		error(1, errno, "Failed to set TCP socket options\n");
	}

	if (setsockopt(dg_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		error(1, errno, "Failed to set UDP socket options\n");
	}

	// bind TCP socket
	if (bind(sockfd, server->ai_addr, server->ai_addrlen) == -1) {
		close(sockfd);
		error(1, errno, "Failed to bind TCP socket.\n");
	}

	// bind UDP socket
	if (bind(dg_sockfd, dg_server->ai_addr, dg_server->ai_addrlen) == -1) {
		close(dg_sockfd);
		error(1, errno, "Failed to bind UDP socket\n");
	}

	freeaddrinfo(dg_server);
	freeaddrinfo(server);

	if (listen(dg_sockfd, BACKLOG) == -1) {
		close(dg_sockfd);
		error(1, errno, "Listen failed (UDP).\n");
	}

	if (listen(sockfd, BACKLOG) == -1) {
		close(sockfd);
		error(1, errno, "Listen failed (TCP).\n");
	}

	users = userhash_create();
	while (!done) {
		destfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);

		// request user name
		send_short(1, dg_sockfd, (struct sockaddr *)&client_addr, addrlen);
		size = recv_short(dg_sockfd, (struct sockaddr *)&client_addr, &addrlen);
		if (recvfrom(dg_sockfd, user, size, 0, (struct sockaddr *)&client_addr, &addrlen) < 0) {
			error(1, errno, "Bad read (user)\n");
		}

		// send 1 if existing user, 0 if not
		uinfo = userinfo_find(users, user);
		send_short((uinfo) ? 1 : 0, dg_sockfd, (struct sockaddr *)&client_addr, addrlen);

		// read in a password
		size = recv_short(dg_sockfd, (struct sockaddr *)&client_addr, &addrlen);
		if (recvfrom(dg_sockfd, pass, size, 0, (struct sockaddr *)&client_addr, &addrlen) < 0) {
			error(1, errno, "Bad read (pass)\n");
		}

		// if user is found and password incorrect, send -1
		// otherwise send 1
		if (uinfo && strcmp(pass, uinfo->pass)) {
			send_short(-1, dg_sockfd, (struct sockaddr *)&client_addr, addrlen);
			break;
		} else if (!uinfo) {
			userhash_add(users, user, pass);
		}
		send_short(1, dg_sockfd, (struct sockaddr *)&client_addr, addrlen);

		while (!done) {
	
			switch (cmd) {
				case CMD_XIT:
					// this shouldn't happen on the server
					break;
				case CMD_CRT:
					break;
				case CMD_MSG:
					break;
				case CMD_DLT:
					break;
				case CMD_EDT:
					break;
				case CMD_LIS:
					break;
				case CMD_RDB:
					break;
				case CMD_APN:
					break;
				case CMD_DWN:
					break;
				case CMD_DST:
					break;
				case CMD_SHT:
					break;
				default:
					break;
			}
		}

		userhash_free(users);
		close(destfd);
	}

}

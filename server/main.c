#include <error.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../common/cmddef.h"
#include "../common/transfer.h"
#include "userhash.h"
#include "cmds.h"

#define BACKLOG 10

int main(int argc, char **argv) 
{
	char *port, *adminpass;
	struct addrinfo hints, *dg_server, *server;
	struct sockaddr_in *client_addr;
	struct sockaddr_in dg_addr;
	struct sockaddr_in dg_client_addr;
	int sockfd, destfd, dg_sockfd;
	int yes = 1;
	uint16_t cmd;
	ssize_t xfer_sz;
	userhash users;
	socklen_t addrlen;
	int done = 0;
	char user[20], pass[20];
	char cmdstr[4];
	uint16_t size;
	userinfo *uinfo = NULL;

	if (argc != 3) {
		error(1, 0, "Usage: %s [port] [admin-pass]\n", argv[0]);
	}

	port = argv[1];
	adminpass = argv[2];

	memset(user, 0, sizeof(user));
	memset(pass, 0, sizeof(pass));
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// resolve TCP addr
	if (getaddrinfo(NULL, port, &hints, &server)) {
		error(1, 0, "Failed to get addr info.\n");
	}

	// resolve UDP addr
	/*hints.ai_socktype = SOCK_DGRAM;
	if (getaddrinfo(NULL, port, &hints, &dg_server)) {
		error(1, 0, "Failed to get UDP addr info.\n");
	}*/
	dg_addr.sin_family = AF_INET;
	dg_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	dg_addr.sin_port = htons(atoi(port));

	// open TCP socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		error(1, errno, "Failed to open TCP socket.\n");
	}

	// open UDP socket
	if ((dg_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		error(1, errno, "Failed to open UDP socket.\n");
	}

	// allow sockets to be reused
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		error(1, errno, "Failed to set TCP socket options\n");
	}

	/*if (setsockopt(dg_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		error(1, errno, "Failed to set UDP socket options\n");
	}*/

	// bind TCP socket
	if (bind(sockfd, server->ai_addr, server->ai_addrlen) == -1) {
		close(sockfd);
		error(1, errno, "Failed to bind TCP socket.\n");
	}

	// bind UDP socket
	if (bind(dg_sockfd, (struct sockaddr *)&dg_addr, sizeof(dg_addr)) == -1) {
		close(dg_sockfd);
		error(1, errno, "Failed to bind UDP socket\n");
	}

	//freeaddrinfo(dg_server);
	//freeaddrinfo(server);

	/*if (listen(dg_sockfd, BACKLOG) == -1) {
		close(dg_sockfd);
		error(1, errno, "Listen failed (UDP).\n");
	}*/

	if (listen(sockfd, BACKLOG) == -1) {
		close(sockfd);
		error(1, errno, "Listen failed (TCP).");
	}

	printf("listening\n");
	users = userhash_create();
	while (!done) {
		destfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
		printf("got connection\n");

		// request user name
		//send_short(1, dg_sockfd, (struct sockaddr *)&dg_addr, addrlen);
		//printf("sent request\n");
		size = recv_short(dg_sockfd, (struct sockaddr *)&dg_client_addr, &addrlen);
		printf("got size of username %i\n", size);
		if (recvfrom(dg_sockfd, user, size, 0, (struct sockaddr *)&dg_client_addr, &addrlen) < 0) {
			error(1, errno, "Bad read (user)");
		}

		printf("got username %s\n", user);

		// send 1 if existing user, 0 if not
		uinfo = userhash_find(users, user);
		//send_short((uinfo) ? 1 : 0, dg_sockfd, (struct sockaddr *)&dg_addr, addrlen);

		// read in a password
		size = recv_short(dg_sockfd, (struct sockaddr *)&dg_client_addr, &addrlen);
		printf("got size of password %i\n", size);
		if (recvfrom(dg_sockfd, pass, size, 0, (struct sockaddr *)&dg_client_addr, &addrlen) < 0) {
			error(1, errno, "Bad read (pass)");
		}

		printf("got password %s\n", pass);

		if (uinfo && strcmp(pass, uinfo->pass)) {
			send_short(-1, dg_sockfd, (struct sockaddr *)&client_addr, addrlen);
			break;
		} else if (!uinfo) {
			userhash_add(users, user, pass);
		}

		while (!done) {
			//cmd = recv_short(dg_sockfd, (struct sockaddr *)&client_addr, &addrlen);
			memset(cmdstr, 0, sizeof(cmdstr));
			if (recvfrom(dg_sockfd, cmdstr, 4, 0, (struct sockaddr *)&dg_addr, &addrlen) < 1) {
				error(1, 0, "bad read (cmd)");
			}
			printf("got cmd %s\n", cmdstr);
			if (!strcmp(cmdstr, CMDSTR_CRT)) {
				create_board(user, dg_sockfd, (struct sockaddr *)&dg_addr, addrlen);
			} else if (!strcmp(cmdstr, CMDSTR_MSG)) {
				message_board(user, dg_sockfd, (struct sockaddr *)&dg_addr, addrlen);
			} else if (!strcmp(cmdstr, CMDSTR_DLT)) {
				delete_message(user, dg_sockfd, (struct sockaddr *)&dg_addr, addrlen);
			} else if (!strcmp(cmdstr, CMDSTR_EDT)) {
				edit_message(user, dg_sockfd, (struct sockaddr *)&dg_addr, addrlen);
			}
		}

		userhash_free(users);
		close(destfd);
	}

	return 0;
}

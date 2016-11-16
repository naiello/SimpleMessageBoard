#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h>
#include <string.h>

#include "../common/transfer.h"
#include "cmds.h"

void create_board(const char *user, int dg_sockfd, struct sockaddr *addr, socklen_t len) {
	uint16_t size;
	char boardname[24];

	memset(boardname, 0, sizeof(boardname));

	size = recv_short(dg_sockfd, addr, &len);
	printf("got boardname length %i\n", size);
	if (recvfrom(dg_sockfd, boardname, size+1, 0, addr, &len) < 0) {
		error(1, errno, "Bad read (CRT)\n");
	}
	printf("got boardname %s\n", boardname);
	strcat(boardname, ".brd");

	if (access(boardname, F_OK) == 0) {
		send_short(0, dg_sockfd, addr, len);
		printf("board already exists\n");
	}

	FILE *file = fopen(boardname, "w");
	fprintf(file, "%s created this board\n", user);
	fclose(file);
	printf("success!\n");
	send_short(1, dg_sockfd, addr, len);
}

void message_board(const char *user, int dg_sockfd, struct sockaddr *addr, socklen_t len) {
	uint16_t size;
	char boardname[24];
	char message[4096];

	memset(boardname, 0, sizeof(boardname));
	memset(message, 0, sizeof(message));
	size = recv_short(dg_sockfd, addr, &len);
	printf("board name size %i\n", size);
	if (recvfrom(dg_sockfd, boardname, size, 0, addr, &len) < 0) {
		error(1, errno, "Bad read (MSG)");
	}
	printf("board name %s\n", boardname);
	strcat(boardname, ".brd");

	size = recv_short(dg_sockfd, addr, &len);
	printf("message size %i\n", size);
	if (recvfrom(dg_sockfd, message, size, 0, addr, &len) < 0) {
		error(1, errno, "bad read (MSG)");
	}

	FILE *file = fopen(boardname, "a");
	if (!file) {
		printf("Failed\n");
		send_short(0, dg_sockfd, addr, len);
		return;
	}

	fprintf(file, "%s: %s\n", user, message);
	fclose(file);
	send_short(1, dg_sockfd, addr, len);
	printf("success!\n");
}

void delete_message(const char *user, int dg_sockfd, struct sockaddr *addr, socklen_t len) {
	uint16_t size, msgind;
	char boardname[24];
	char boardname2[28];
	char line[4096];

	memset(boardname, 0, sizeof(boardname));
	memset(boardname2, 0, sizeof(boardname2));
	memset(line, 0, sizeof(line));
	size = recv_short(dg_sockfd, addr, &len);
	printf("boardname size %i\n", size);
	if (recvfrom(dg_sockfd, boardname, size, 0, addr, &len) < 0) {
		error(1, 0, "Bad read (DLT)");
	}
	printf("original board %s\n", boardname);

	msgind = recv_short(dg_sockfd, addr, &len);
	printf("msgind %i\n", msgind);

	strcat(boardname, ".brd");
	printf("boardname %s\n", boardname);
	strcpy(boardname2, boardname);
	strcat(boardname2, ".tmp");
	FILE *file = fopen(boardname, "r");
	if (!file) {
		printf("failed\n");
		send_short(0, dg_sockfd, addr, len);
	}
	FILE *file2 = fopen(boardname2, "w");
	printf("opened files!\n");

	int i = 0;
	int success = 0;
	while (fgets(line, sizeof(line), file) != NULL) {
		if ((i != msgind) || !(strstr(line, user) == line)) {
			fprintf(file2, "%s", line);
		} else {
			success = 1;
		}
		i++;
	}
	fclose(file);
	fclose(file2);
	printf("success? %i\n", success);
	unlink(boardname);
	rename(boardname2, boardname);
	send_short(success, dg_sockfd, addr, len);
}

void edit_message(const char *user, int dg_sockfd, struct sockaddr *addr, socklen_t len) {
	uint16_t size, msgind, msgsz;
	char boardname[24];
	char boardname2[28];
	char msg[4096];
	char line[4096];

	memset(boardname, 0, sizeof(boardname));
	memset(boardname2, 0, sizeof(boardname2));
	memset(msg, 0, sizeof(msg));
	memset(line, 0, sizeof(line));
	size = recv_short(dg_sockfd, addr, &len);
	printf("boardname size %i\n", size);
	if (recvfrom(dg_sockfd, boardname, size, 0, addr, &len) < 0) {
		error(1, 0, "Bad read (EDT)");
	}
	printf("boardname %s\n", boardname);

	msgind = recv_short(dg_sockfd, addr, &len);
	printf("msgind %i\n", msgind);

	strcat(boardname, ".brd");
	strcpy(boardname2, boardname);
	strcat(boardname2, ".tmp");

	msgsz = recv_short(dg_sockfd, addr, &len);
	printf("message size %i\n", msgsz);
	if (recvfrom(dg_sockfd, msg, msgsz, 0, addr, &len) < 0) {
		error(1, 0, "bad read (EDT)");
	}
	printf("message %s\n", msg);

	FILE *file = fopen(boardname, "r");
	if (!file) {
		printf("failed\n");
		send_short(-1, dg_sockfd, addr, len);
	}
	FILE *file2 = fopen(boardname2, "w");
	printf("opened files!\n");

	int i = 0;
	int success = 0;
	while (fgets(line, sizeof(line), file) != NULL) {
		if ((i != msgind) || !(strstr(line, user) == line)) {
			fprintf(file2, "%s", line);
		} else {
			success = 1;
			fprintf(file2, "%s: %s\n", user, msg);
		}
		i++;
	}
	fclose(file);
	fclose(file2);
	printf("success? %i\n", success);
	unlink(boardname);
	rename(boardname2, boardname);
	send_short(success, dg_sockfd, addr, len);
}

void list_boards(int dg_sockfd, struct sockaddr *addr, socklen_t len) {
	char buffer[4096];
	DIR *d;
	struct dirent *dir;

	memset(buffer, 0, sizeof(buffer));

	d = opendir(".");
	while ((dir = readdir(d))) {
		if (strstr(dir->d_name, ".brd")) {
			strcat(buffer, dir->d_name);
			strcat(buffer, "\n");
		}
	}

	send_long(strlen(buffer), dg_sockfd, addr, len);
	if (sendto(dg_sockfd, buffer, strlen(buffer), 0, addr, len) < 0) {
		error(1, 0, "bad send (lis)");
	}
}

void read_board(int sockfd) {
	char boardname[24];
	uint16_t size;

	printf("begin!\n");
	fflush(stdout);
	memset(boardname, 0, sizeof(boardname));
	recv(sockfd, &size, sizeof(size), 0);
	size = ntohs(size);
	printf("board size %i\n", size);
	fflush(stdout);
	if (recv(sockfd, boardname, size, 0) < 0) {
		error(1, errno, "bad recv (rdb)");
	}
	strcat(boardname, ".brd");
	printf("board %s\n", boardname);
	fflush(stdout);

	send_file(sockfd, boardname);
}

void append_file(const char *user, int sockfd) {
	char boardname[40];
	char boardname_full[24];
	char filename[20];
	uint16_t bsize, fnsize, ack;

	memset(boardname, 0, sizeof(boardname));
	memset(filename, 0, sizeof(filename));
	memset(boardname_full, 0, sizeof(filename));
	if ((recv(sockfd, &bsize, sizeof(bsize), 0) < 0) ||
			(recv(sockfd, boardname, bsize = ntohs(bsize), 0) < 0) ||
			(recv(sockfd, &fnsize, sizeof(fnsize), 0) < 0) ||
			(recv(sockfd, filename, fnsize = ntohs(fnsize), 0)) < 0) {
		error(1, 0, "bad recv (APN)");
	}

	strcpy(boardname_full, boardname);
	strcat(boardname_full, ".brd");
	if (access(boardname_full, F_OK) != 0) {
		ack = 0;
		send(sockfd, &ack, sizeof(ack), 0);
		return;
	}

	char *slashpos = filename, *i = filename;
	do {
		if (*i == '/') slashpos = i+1;
	} while (*(++i) != 0);

	strcat(boardname, "-");
	strcat(boardname, slashpos);
	strcat(boardname, ".att");
	printf("file name %s\n", boardname);
	
	recv_file(sockfd, boardname);

	FILE *file = fopen(boardname_full, "a");
	fprintf(file, "%s appended the file %s\n", user, slashpos);
	fclose(file);
}

void download_file(int sockfd) {
	char boardname[40];
	char boardname_full[24];
	char filename[20];
	uint16_t bsize, fnsize, ack;

	memset(boardname, 0, sizeof(boardname));
	memset(filename, 0, sizeof(filename));
	memset(boardname_full, 0, sizeof(filename));
	if ((recv(sockfd, &bsize, sizeof(bsize), 0) < 0) ||
			(recv(sockfd, boardname, bsize, 0) < 0) ||
			(recv(sockfd, &fnsize, sizeof(fnsize), 0) < 0) ||
			(recv(sockfd, filename, fnsize, 0)) < 0) {
		error(1, 0, "bad recv (APN)");
	}

	strcpy(boardname_full, boardname);
	strcat(boardname_full, ".brd");
	strcat(boardname, "-");
	strcat(boardname, filename);
	strcat(boardname, ".att");
	printf("file name %s\n", boardname);
	if ((access(boardname_full, F_OK) != 0) || (access(boardname, F_OK) != 0)) {
		ack = 0;
		send(sockfd, &ack, sizeof(ack), 0);
		return;
	}

	send_file(sockfd, boardname);
}

void destroy_board(const char *user, int dg_sockfd, struct sockaddr *addr, socklen_t len) {
	char boardname[20];
	char boardname_full[24];
	char line[4096];
	uint16_t size;

	memset(boardname, 0, sizeof(boardname));
	memset(boardname_full, 0, sizeof(boardname_full));
	size = recv_short(dg_sockfd, addr, &len);
	if (recvfrom(dg_sockfd, boardname, size, 0, addr, &len) < 0) {
		error(1, errno, "bad read (dst)");
	}

	strcpy(boardname_full, boardname);
	strcat(boardname_full, ".brd");
	printf("boardname %s\n", boardname);
	FILE *file = fopen(boardname, "r");
	if (!file) {
		printf("does not exist\n");
		send_short(0, dg_sockfd, addr, len);
		return;
	}
	fgets(line, sizeof(line), file);
	if (strstr(line, user) != line) {
		printf("not same user\n");
		send_short(0, dg_sockfd, addr, len);
		return;
	}

	fclose(file);
	DIR *d = opendir(".");
	struct dirent *dir;
	while ((dir = readdir(d)) != NULL) {
		if (strstr(boardname, dir->d_name) != NULL) {
			unlink(dir->d_name);
		}
	}
	send_short(1, dg_sockfd, addr, len);
}

void shutdown_server(const char *pwd, int dg_sockfd, struct sockaddr *addr, socklen_t len) {
	char pass[20];
	uint16_t psize;

	memset(pass, 0, sizeof(pass));
	psize = recv_short(dg_sockfd, addr, &len);
	printf("psize %i\n", psize);
	if (recvfrom(dg_sockfd, pass, psize, 0, addr, &len) == -1) {
		error(1, errno, "bad read (SHT)");
	}
	printf("password %s\n", pass);

	if (strcmp(pwd, pass)) {
		printf("wrong password - %s\n", pwd);
		send_short(0, dg_sockfd, addr, len);
		return;
	}
	printf("accepted\n");

	DIR *d = opendir(".");
	struct dirent *dir;
	while ((dir = readdir(d)) != NULL) {
		if ((strstr(dir->d_name, ".att") != NULL) || 
			((strstr(dir->d_name, ".brd")) != NULL)){
			unlink(dir->d_name);
		}
	}
	printf("done\n");

	send_short(1, dg_sockfd, addr, len);
	exit(0);
}

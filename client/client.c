/* Simple Message Board
 * Client code
 * Rosalyn Tan & Nick Aiello
 * rtan, naiello
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "../common/transfer.h"

#define MAX_LINE 4096

void display_menu();
void cmd_crt(int, char*, struct sockaddr_in);
void cmd_msg(int, char*, struct sockaddr_in);
void cmd_dlt(int, char*, struct sockaddr_in);
void cmd_edt(int, char*, struct sockaddr_in);
void cmd_lis(int, char*, struct sockaddr_in);
void cmd_rdb(int, int, char*, struct sockaddr_in);
void cmd_apn(int, int, char*, struct sockaddr_in);
void cmd_dwn(int, int, char*, struct sockaddr_in);
void cmd_dst(int, char*, struct sockaddr_in);
int cmd_sht(int, int, char*, struct sockaddr_in);

int main(int argc, char** argv) {
	char* host;
	int portNum;
	char* portChar;
	int udpsock;
	int tcpsock;	
	struct hostent* hp;
	struct sockaddr_in sin;
	struct addrinfo hints, *serv_addr;
	socklen_t addr_len;
	short int ack;
	char username[20];
	char password[20];
	char cmd[4];

	if(argc != 3) {
		perror("Invalid number of arguments");
		exit(1);
	} else {
		host = argv[1];
		portNum = atoi(argv[2]);
		portChar = argv[2];
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

	if(getaddrinfo(host, portChar, &hints, &serv_addr) != 0) {
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

	bzero((char*)username, sizeof(username));
	printf("Enter username: ");
	scanf("%s", username);

	addr_len = sizeof(struct sockaddr);

	// wait for server to request username
	if(recvfrom(udpsock, &ack, sizeof(ack), 0, (struct sockaddr*)&sin, &addr_len) == -1) {
		perror("Receive ack failed");
		exit(1);
	} else {
		printf("Username requested\n");
	}

	short int user_len = htons(strlen(username));
	// send username length
	if(sendto(udpsock, &user_len, sizeof(user_len), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Username size send error");
		exit(1);
	}

	// send username to server
	if(sendto(udpsock, username, strlen(username) + 1, 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Client username send error");
		exit(1);
	}
	
	bzero((char*)password, sizeof(password));
	printf("Enter password: ");
	scanf("%s", password);

	if(recvfrom(udpsock, &ack, sizeof(ack), 0, (struct sockaddr*)&sin, &addr_len) == -1) {
		perror("Receive ack failed");
		exit(1);
	}

	short int pass_len = htons(strlen(password));
	if(sendto(udpsock, &pass_len, sizeof(pass_len), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Send password length error");
		exit(1);
	}

	// send password to server
	if(sendto(udpsock, password, strlen(password) + 1, 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Client password send error");
		exit(1);
	}

	int i;
	bzero((char*)cmd, sizeof(cmd));
	while(strcmp(cmd, "XIT")) {
		display_menu();
		scanf("%s", cmd);
		for(i = 0; i < strlen(cmd); i++ ) {
			cmd[i] = toupper(cmd[i]);
		}
		cmd[3] = '\0';
		if(!strcmp(cmd, "CRT")) {
			cmd_crt(udpsock, cmd, sin);
		} else if(!strcmp(cmd, "LIS")) {
			cmd_lis(udpsock, cmd, sin);
		} else if(!strcmp(cmd, "MSG")) {
			cmd_msg(udpsock, cmd, sin);
		} else if(!strcmp(cmd, "DLT")) {
			cmd_dlt(udpsock, cmd, sin);
		} else if(!strcmp(cmd, "RDB")) {
			cmd_rdb(tcpsock, udpsock, cmd, sin);
		} else if(!strcmp(cmd, "EDT")) {
			cmd_edt(udpsock, cmd, sin);
		} else if(!strcmp(cmd, "APN")) {
			cmd_apn(tcpsock, udpsock, cmd, sin);
		} else if(!strcmp(cmd, "DWN")) {
			cmd_dwn(tcpsock, udpsock, cmd, sin);
		} else if(!strcmp(cmd, "DST")) {
			cmd_dst(udpsock, cmd, sin);
		} else if(!strcmp(cmd, "XIT")) {
			continue;
		} else if(!strcmp(cmd, "SHT")) {
			if(cmd_sht(tcpsock, udpsock, cmd, sin) == 1) {
				break;
			}
		} else {
			printf("Please enter a valid option\n");
		}
		
	}
	return 0;	
}

void display_menu() {
	printf("Please select one of the following options:\n");
	printf("CRT: Create Board\nLIS: List Boards\nMSG: Leave Message\nDLT: Delete Message\nRDB: Read Board\nEDT: Edit Message\nAPN: Append File\nDWN: Download File\nDST: Destroy Board\nXIT: Exit\nSHT: Shutdown Server\n");
}

void cmd_crt(int udpsock, char* cmd, struct sockaddr_in sin) {
	char board_name[20];
	short int ack;

	bzero((char*)board_name, sizeof(board_name));

	if(sendto(udpsock, cmd, sizeof(cmd), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending command");
		return;
	}

	getc(stdin);
	printf("Enter a board name: ");
	scanf("%[^\n]", board_name);

	// send length of board name
	short int bname_len = htons(strlen(board_name));
	if(sendto(udpsock, &bname_len, sizeof(bname_len), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending board name length");
		return;
	}
	
	if(sendto(udpsock, board_name, strlen(board_name) + 1, 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending board name");
		return;
	}

	socklen_t addr_len = sizeof(struct sockaddr);

	if(recvfrom(udpsock, &ack, sizeof(ack), 0, (struct sockaddr*)&sin, &addr_len) == -1) {
		perror("Error receiving");
		return;
	} else {
		ack = ntohs(ack);
		if(ack == 1) {
			printf("Board successfully created\n");
		} else {
			printf("Error creating board\n");
		}
	}
}

void cmd_msg(int udpsock, char* cmd, struct sockaddr_in sin) {
	char board_name[20];
	short int ack;
	char message[MAX_LINE];

	bzero((char*)board_name, sizeof(board_name));
	bzero((char*)message, sizeof(message));

	if(sendto(udpsock, cmd, sizeof(cmd), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending command");
		return;
	}

	getc(stdin);
	printf("Enter a board name: ");
	scanf("%[^\n]", board_name);

	// send length of board name
	short int bname_len = htons(strlen(board_name));
	if(sendto(udpsock, &bname_len, sizeof(bname_len), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending board name length");
		return;
	}
	
	if(sendto(udpsock, board_name, strlen(board_name) + 1, 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending board name");
		return;
	}

	getc(stdin);
	printf("Enter a message: ");
	scanf("%[^\n]", message);

	// send length of message
	short int message_len = htons(strlen(message));
	if(sendto(udpsock, &message_len, sizeof(message_len), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending message length");
		return;
	}
	
	if(sendto(udpsock, message, strlen(message) + 1, 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending message");
		return;
	}
	
	socklen_t addr_len = sizeof(struct sockaddr);

	if(recvfrom(udpsock, &ack, sizeof(ack), 0, (struct sockaddr*)&sin, &addr_len) == -1) {
		perror("Error receiving");
		return;
	} else {
		ack = ntohs(ack);
		if(ack == 1) {
			printf("Message successfully posted\n");
		} else {
			printf("Board does not exist\n");
		}
	}
}

void cmd_dlt(int udpsock, char* cmd, struct sockaddr_in sin) {
	char board_name[20];
	short int ack;
	short int del;

	bzero((char*)board_name, sizeof(board_name));

	if(sendto(udpsock, cmd, sizeof(cmd), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending command");
		return;
	}

	getc(stdin);
	printf("Enter a board name: ");
	scanf("%[^\n]", board_name);

	// send length of board name
	short int bname_len = htons(strlen(board_name));
	if(sendto(udpsock, &bname_len, sizeof(bname_len), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending board name length");
		return;
	}
	
	if(sendto(udpsock, board_name, strlen(board_name) + 1, 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending board name");
		return;
	}

	printf("Which message do you want to be deleted? Enter a number: ");
	scanf("%hd", &del);
	del = htons(del);
	
	if(sendto(udpsock, &del, sizeof(del), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending message number");
		return;
	}

	socklen_t addr_len = sizeof(struct sockaddr);

	if(recvfrom(udpsock, &ack, sizeof(ack), 0, (struct sockaddr*)&sin, &addr_len) == -1) {
		perror("Error receiving");
		return;
	} else {
		ack = ntohs(ack);
		if(ack == 1) {
			printf("Message successfully deleted\n");
		} else {
			printf("Error deleting message\n");
		}
	}
}

void cmd_edt(int udpsock, char* cmd, struct sockaddr_in sin) {
	char board_name[20];
	char message[MAX_LINE];
	short int del;
	short int ack;
	
	bzero((char*)message, sizeof(message));

	if(sendto(udpsock, cmd, sizeof(cmd), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending command");
		return;
	}

	getc(stdin);
	printf("Enter a board name: ");
	scanf("%[^\n]", board_name);

	// send length of board name
	short int bname_len = htons(strlen(board_name));
	if(sendto(udpsock, &bname_len, sizeof(bname_len), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending board name length");
		return;
	}
	
	if(sendto(udpsock, board_name, strlen(board_name) + 1, 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending board name");
		return;
	}

	printf("Which message do you want to be edited? Enter a number: ");
	scanf("%hd", &del);
	del = htons(del);

	if(sendto(udpsock, &del, sizeof(del), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending message number");
		return;
	}

	getc(stdin);
	printf("What do you want your new message to be?\n");
	scanf("%[^\n]", message);

	short int message_len = htons(strlen(message));

	if(sendto(udpsock, &message_len, sizeof(message_len), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending message length");
		return;
	}
	
	if(sendto(udpsock, message, strlen(message) + 1, 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending new message");
		return;
	}

	socklen_t addr_len = sizeof(struct sockaddr);

	if(recvfrom(udpsock, &ack, sizeof(ack), 0, (struct sockaddr*)&sin, &addr_len) == -1) {
		perror("Error receiving");
		return;
	} else {
		ack = ntohs(ack);
		if(ack == 1) {
			printf("Message successfully edited\n");
		} else {
			printf("Error editing message\n");
		}
	}
}

void cmd_lis(int udpsock, char* cmd, struct sockaddr_in sin) {
	int list_len;
	char listing[MAX_LINE];
	socklen_t addr_len;

	bzero((char*)listing, sizeof(listing));

	if(sendto(udpsock, cmd, sizeof(cmd), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending command");
		return;
	}

	if(recvfrom(udpsock, &list_len, sizeof(list_len), 0, (struct sockaddr*)&sin, &addr_len) == -1) {
		perror("Error receiving listing length");
		return;
	}

	if(recvfrom(udpsock, listing, MAX_LINE, 0, (struct sockaddr*)&sin, &addr_len) == -1) {
		perror("Error receiving listing");
		return;
	}

	printf("%s\n", listing);
}

void cmd_rdb(int tcpsock, int udpsock, char* cmd, struct sockaddr_in sin) {
	char board_name[20];

	bzero((char*)board_name, sizeof(board_name));
	
	if(sendto(udpsock, cmd, sizeof(cmd), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending command");
		return;
	}

	getc(stdin);
	printf("Enter a board name: ");
	scanf("%[^\n]", board_name);

	// send length of board name
	short int bname_len = htons(strlen(board_name));
	if(write(tcpsock, &bname_len, sizeof(bname_len)) == -1) {
		perror("Error sending board name length");
		return;
	}
	
	if(write(tcpsock, board_name, strlen(board_name) + 1) == -1) {
		perror("Error sending board name");
		return;
	}

	if(recv_file_print(tcpsock) < 0) {
		perror("Error receiving board");
	}
}

void cmd_apn(int tcpsock, int udpsock, char* cmd, struct sockaddr_in sin) {
	char board_name[20];
	char file_name[20];
	short int ack;

	bzero((char*)board_name, sizeof(board_name));
	
	if(sendto(udpsock, cmd, sizeof(cmd), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending command");
		return;
	}

	getc(stdin);
	printf("Enter a board name: ");
	scanf("%[^\n]", board_name);

	// send length of board name
	short int bname_len = htons(strlen(board_name));
	if(write(tcpsock, &bname_len, sizeof(bname_len)) == -1) {
		perror("Error sending board name length");
		return;
	}
	
	if(write(tcpsock, board_name, strlen(board_name) + 1) == -1) {
		perror("Error sending board name");
		return;
	}

	getc(stdin);
	printf("Enter a file name: ");
	scanf("%[^\n]", file_name);

	// send length of file name
	short int fname_len = htons(strlen(file_name));
	if(write(tcpsock, &fname_len, sizeof(fname_len)) == -1) {
		perror("Error sending file name length");
		return;
	}
	
	if(write(tcpsock, file_name, strlen(file_name) + 1) == -1) {
		perror("Error sending file name");
		return;
	}

	if(read(tcpsock, &ack, sizeof(ack)) == -1) {
		perror("Error receiving ack");
		return;
	} else {
		if(ack == 0) {
			return;
		}
	}
	send_file(tcpsock, file_name);
}

void cmd_dst(int udpsock, char* cmd, struct sockaddr_in sin) {
	char board_name[20];
	short int ack;

	bzero((char*)board_name, sizeof(board_name));

	if(sendto(udpsock, cmd, sizeof(cmd), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending command");
		return;
	}

	getc(stdin);
	printf("Enter a board name: ");
	scanf("%[^\n]", board_name);

	// send length of board name
	short int bname_len = htons(strlen(board_name));
	if(sendto(udpsock, &bname_len, sizeof(bname_len), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending board name length");
		return;
	}
	
	if(sendto(udpsock, board_name, strlen(board_name) + 1, 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending board name");
		return;
	}

	socklen_t addr_len = sizeof(struct sockaddr);

	if(recvfrom(udpsock, &ack, sizeof(ack), 0, (struct sockaddr*)&sin, &addr_len) == -1) {
		perror("Error receiving");
		return;
	} else {
		ack = ntohs(ack);
		if(ack == 1) {
			printf("Board successfully destroyed\n");
		} else {
			printf("Error destroying board\n");
		}
	}
}

void cmd_dwn(int tcpsock, int udpsock, char* cmd, struct sockaddr_in sin) {
	char board_name[20];
	char file_name[20];

	bzero((char*)board_name, sizeof(board_name));
	
	if(sendto(udpsock, cmd, sizeof(cmd), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending command");
		return;
	}

	getc(stdin);
	printf("Enter a board name: ");
	scanf("%[^\n]", board_name);

	// send length of board name
	short int bname_len = htons(strlen(board_name));
	if(write(tcpsock, &bname_len, sizeof(bname_len)) == -1) {
		perror("Error sending board name length");
		return;
	}
	
	if(write(tcpsock, board_name, strlen(board_name) + 1) == -1) {
		perror("Error sending board name");
		return;
	}

	getc(stdin);
	printf("Enter a file name: ");
	scanf("%[^\n]", file_name);

	// send length of file name
	short int fname_len = htons(strlen(file_name));
	if(write(tcpsock, &fname_len, sizeof(fname_len)) == -1) {
		perror("Error sending file name length");
		return;
	}
	
	if(write(tcpsock, file_name, strlen(file_name) + 1) == -1) {
		perror("Error sending file name");
		return;
	}

	recv_file(tcpsock, file_name);
}

int cmd_sht(int tcpsock, int udpsock, char* cmd, struct sockaddr_in sin) {
	char password[20];
	short int ack;

	bzero((char*)password, sizeof(password));

	if(sendto(udpsock, cmd, sizeof(cmd), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending command");
		return -1;
	}

	printf("Enter the admin password: ");
	scanf("%s", password);

	if(sendto(udpsock, password, strlen(password) + 1, 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1) {
		perror("Error sending password");
		return -1;
	}

	socklen_t addr_len = sizeof(struct sockaddr);

	if(recvfrom(udpsock, &ack, sizeof(ack), 0, (struct sockaddr*)&sin, &addr_len) == -1) {
		perror("Error receiving ack");
		return -1;
	} else {
		ack = ntohs(ack);
		if(ack == 1) {
			close(udpsock);
			close(tcpsock);
			return 1;
		} else {
			return 0;
		}
	}
}

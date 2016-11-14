#include <error.h>
#include <stdio.h>
#include <sys/socket.h>

#include "../common/cmddef.h"

int main(int argc, char **argv) 
{
	char *port, adminpass;
	if (argc != 3) {
		error(1, 0, "Usage: %s [port] [admin-pass]\n", argv[0]);
	}

	port = argv[1];
	adminpass = argv[2];
}

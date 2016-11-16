#ifndef TRANSFER_H_
#define TRANSFER_H_

#include <sys/socket.h>

#define XFER_BUFSZ 4096

ssize_t send_file(int, const char *);
ssize_t recv_file(int, const char *);
ssize_t recv_file_print(int);

ssize_t send_short(uint16_t, int, struct sockaddr *, socklen_t);
ssize_t send_long(uint32_t, int, struct sockaddr *, socklen_t);
uint16_t recv_short(int, struct sockaddr *, socklen_t*);
uint32_t recv_long(int, struct sockaddr *, socklen_t*);

#endif

#ifndef TRANSFER_H_
#define TRANSFER_H_

#define XFER_BUFSZ 4096

ssize_t send_file(int, const char *);
ssize_t recv_file(int, const char *);

#endif

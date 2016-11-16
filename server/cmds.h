#ifndef CMDS_H_
#define CMDS_H_

void create_board(const char *, int, struct sockaddr *, socklen_t);
void message_board(const char *, int, struct sockaddr *, socklen_t);
void delete_board(const char *, int, struct sockaddr *, socklen_t);
void edit_message(const char *, int, struct sockaddr *, socklen_t);
void list_board(int, struct sockaddr *, socklen_t);
void read_board(int, int, struct sockaddr *, socklen_t);
void append_file(int, int, struct sockaddr *, socklen_t);
void download_file(int, int, struct sockaddr *, socklen_t);
void destroy_board(int, struct sockaddr *, socklen_t);

#endif

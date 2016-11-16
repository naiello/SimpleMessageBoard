#ifndef CMDS_H_
#define CMDS_H_

void create_board(const char *, int, struct sockaddr *, socklen_t);
void message_board(const char *, int, struct sockaddr *, socklen_t);
void delete_message(const char *, int, struct sockaddr *, socklen_t);
void edit_message(const char *, int, struct sockaddr *, socklen_t);
void list_boards(int, struct sockaddr *, socklen_t);
void read_board(int);
void append_file(const char *, int);
void download_file(int, int, struct sockaddr *, socklen_t);
void destroy_board(const char *, int, struct sockaddr *, socklen_t);

#endif

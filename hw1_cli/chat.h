#ifndef CHAT_H
#define CHAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/select.h>

#define MAXLINE 5120

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

#define SENT_MSG_C "\x1B[1;32m"
#define RECV_MSG_C "\x1B[1;33m"
#define DEFAULT_C "\x1B[0m"

void error(char *msg);
void write_to_screen(int fd, char *buf, char *arrow, char *color);


#endif
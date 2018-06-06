#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <stdarg.h>

#define MAXLINE 5120

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

#define	SA	struct sockaddr
/*Color Defines*/
#define C_VERBOSE "\x1B[1;34m"
#define C_ERRORS "\x1B[1;31m"
#define C_DEFAULT "\x1B[0m"
#define C_INFO "\x1B[1;33m"

#define CLIENT_LOGIN_INIT "ME2U"
#define SERVER_LOGIN_INIT "U2EM"
#define CLIENT_SET_USER "IAM"
#define SERVER_USER_TAKEN "ETAKEN"
#define USERNAME_TAKEN "Username Taken"
#define SERVER_USER_APPROVE "MAI"
#define SERVER_MOTD "MOTD"

#define CLIENT_LIST_USER "LISTU"
#define SERVER_LIST_USER "UTSIL"

#define MSG_TO "TO"
#define CONFIRM_MSG_TO "OT"
#define MSG_FROM "FROM"
#define CONFIRM_MSG_FROM "MORF"
#define NO_USR "EDNE"

#define CLIENT_BYE "BYE"
#define SERVER_BYE_ACK "EYB"
#define SERVER_BYE_BROD "UOFF"

#define ENDING "\r\n\r\n"

#define USER_HELP "/help"
#define USER_LOGOUT "/logout"
#define USER_LISTU "/listu"
#define USER_CHAT "/chat"

// char CURRENT_USER_NAME[20];
extern char CURRENT_USER_NAME[20];

typedef struct {
    char *username;
    int to_cnt;
    int ot_cnt;
    int read_fd;
    int write_fd;
} user_struct;

struct list_user {
    user_struct* curr_user;
    int pid;
    struct list_user* next;
};
typedef struct list_user list_user;

list_user *curr_list;

/*Basic print stuff*/
void print_usage(int exit_code);
void print_verbose();
void print_error(char *format, ...);
void printf_error(FILE *stream, char *format, ...);
void print_info(char *format, ...);

void error(char *msg); /*This error exits program*/

/*Color chaning functions*/
void color_verbose();
void color_error();
void color_reset();
void color_info();
/*Basic checks*/
bool valid_username(char* name);

void login_proc(FILE *fp, int sockfd);

char* get_string_without_ending(char *str);
char* set_string_ending(char* str);

void regular_chat(FILE *fp, int sockfd);

void print_client_help();

bool startsWith(char *str, char *pre);

bool find_open_user(char *username);

#endif

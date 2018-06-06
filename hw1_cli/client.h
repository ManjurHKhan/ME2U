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
#include <signal.h>
#include <sys/wait.h>


#define MAXLINE 5120

#define SA struct sockaddr

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

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

typedef struct {
    char *username;
    int to_cnt;
    int ot_cnt;
    int rd;
    int wrt;
    bool active;
} user_struct;

struct list_user {
    user_struct* curr_user;
    pid_t pid;
    struct list_user* next;
    struct list_user* prev;
};
typedef struct list_user list_user;

list_user *user_list_head;
list_user *user_list_tail;


void proper_exit();
void error(char *msg);

void print_usage(int exit_code);
void print_verbose();

void login_proc(FILE *fp, int sockfd);
void regular_chat(FILE *fp, int sockfd);

void send_login_msg(int sockfd);
void send_username(int sockfd);

char* process_string(char *buf);

void increase_full_buf();
void reset_full_buf();

void color_verbose();
void color_reset();

int word_length(char *string);

void list_users(char *user_list);
void msg_confirmed(char *name);
void no_user_exists(char *string);
void someone_exited(char *string);
void process_msg(char *msg);

void print_client_help();
char* set_string_ending(char* str);

void sigchild_handler(int sig);
void any_children_replied(fd_set rset, int sockfd);
user_struct* find_user(char *name);
user_struct* create_new_user(char *name);

list_user* find_user_pid(pid_t pid);

void set_fd_for_childs(fd_set rset);

#endif

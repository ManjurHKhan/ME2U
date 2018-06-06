#include "client.h"

char CURRENT_USER_NAME[];

void print_usage(int exit_code) {
    printf("%s\n", "\
    Printing USAGE\n\
    ./client [-hv] NAME SERVER_IP SERVER_PORT\n\
    -h                  Displays this help menu, and returns EXIT_SUCCESS.\n\
    -v                  Verbose print all incoming and outgoing protocol verbs & content.\n\
    NAME                This is the username to display when chatting.\n\
    SERVER_IP           The ip address of the server to connect to.\n\
    SERVER_PORT         The port to connect to.\
    \
    ");
    exit(exit_code);
}

void print_verbose() {
    color_verbose();
    printf("%s\n", "\
    This is verbose printing\
    \
    ");
    color_reset();
    fflush(stdin);
}

void print_error(char *format, ...) {
    color_error();
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    color_reset();
    fflush(stdin);
}

void printf_error(FILE *stream, char *format, ...) {
    color_error();
    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);
    color_reset();
    fflush(stdin);
}

void print_msg(char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdin);
}
void print_info(char *format, ...) {
    color_info();
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    color_reset();
    fflush(stdin);
}
void color_verbose() {
    printf("%s", C_VERBOSE);
}
void color_error() {
    printf("%s", C_ERRORS);
}
void color_reset() {
    printf("%s", C_DEFAULT);
}
void color_info() {
    printf("%s", C_INFO);
}

void error(char *msg) {
    perror(msg);
    exit(0);
}

bool valid_username(char* name) {
    char* ptr = name;
    if(strlen(name) > 10) {
        return false;
    }
    while(*ptr != '\0') {
        if(*ptr == ' ' || *ptr == '\n') {
            return false;
        }
    }
    return true;
}

char* get_string_without_ending(char *str) {
    // perror("Inside get_string\n");
    // printf_error(stderr, "--%s", ENDING);
    // printf_error(stderr, "%s", str);
    // char *ending = strstr(str, ENDING);
    // // perror("after ending");
    // if(ending == NULL) {
    //     // perror("return null str w/o end\n");
    //     return NULL;
    // }
    // if(ending == str) { // This is when I get null
    //     // perror("return null char\n");
    //     return "\0";
    // }
    // // perror("return something else\n");
    // size_t new_size = strlen(str) - strlen(ending);
    // char *to_ret = calloc(new_size, sizeof(char));
    // memcpy(to_ret, str, new_size);
    char *to_ret = calloc(strlen(str), sizeof(char));
    memcpy(to_ret, str, strlen(str));
    return to_ret;
}

char* set_string_ending(char* str) {
    size_t old_size = strlen(str);
    char* to_ret = calloc(old_size, sizeof(char));
    memcpy(to_ret, str, old_size);
    memcpy(to_ret + old_size, ENDING, 4);
    return to_ret;
}

void login_proc(FILE *fp, int sockfd) {
    // printf_error(stderr, "user name login is =>> %s\n", CURRENT_USER_NAME);
    // print_info("meaw is a cat");
    int maxfdpl, stdineof, n;
    fd_set rset;
    char buf[MAXLINE];

    stdineof = 0;
    FD_ZERO(&rset);
    struct timeval timeout;

    int login_state = 0; // login_state 0 = innitiate login

    while(login_state != 10) { // login_state 10 = done login
        memset(buf, 0, MAXLINE);
        if(stdineof == 0) {
            FD_SET(fileno(fp), &rset);
        }
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        FD_SET(sockfd, &rset);
        maxfdpl = MAX(fileno(fp), sockfd) + 1;
        char* temp_str = NULL;
        if(login_state == 0) {
            temp_str = set_string_ending(CLIENT_LOGIN_INIT);
            write(sockfd, temp_str, strlen(temp_str));
            login_state = 1; // login_state 1 = waiting for server hello
        }
        if(select(maxfdpl, &rset, NULL, NULL, &timeout) != 0) {
            switch(login_state) {
                case 1: // recieve server hello
                    // perror("inside case 1\n");
                    if(FD_ISSET(sockfd, &rset)) {
                        // perror("inside case 1 fd_isset\n");
                        if((n = read(sockfd, buf, MAXLINE)) == 0) {
                            error("Server exited prevaturely while in hello procedure");
                        } else {
                            // perror("1 else here\n");
                            temp_str = get_string_without_ending(buf);
                            if (strlen(temp_str) == 0) {
                                printf_error(stderr, "\n==>%s<<\n", temp_str);
                                break;
                            // } else if(strcmp(temp_str, SERVER_LOGIN_INIT) != 0) {
                            } else if(!startsWith(temp_str, SERVER_LOGIN_INIT)) {
                                error("Expected server hello, got something else");
                            }
                            // perror("login_state 2\n");
                            login_state = 2; // login_state 2 = waiting for username
                        }
                    }
                    break;
                case 2: // send username
                    // printf_error(stderr, "inside login state 2\n");
                    temp_str = calloc(strlen(CURRENT_USER_NAME) + strlen(CLIENT_SET_USER) + 5, sizeof(char));
                    memcpy(temp_str, CLIENT_SET_USER, 3);
                    memcpy(temp_str + 3, " ", 1);
                    memcpy(temp_str + 4, CURRENT_USER_NAME, strlen(CURRENT_USER_NAME));
                    temp_str = set_string_ending(temp_str);
                    write(sockfd, temp_str, strlen(temp_str));
                    login_state = 3; // login_state 3 = waiting for server approval
                    break;
                case 3: // server approval
                    if(FD_ISSET(sockfd, &rset)) {
                        // printf_error(stdin, "didnt read anything yet\n");
                        if((n = read(sockfd, buf, MAXLINE)) == 0) {
                            error("Server exited prevaturely while in username approval procedure");
                        } else {
                            // printf_error(stdin, "got string %s\n", buf);
                            temp_str = get_string_without_ending(buf);
                            // if(strstr(temp_str, SERVER_USER_TAKEN) != NULL) {
                            if(startsWith(temp_str, SERVER_USER_TAKEN)) {
                                // login_state = 5; // re enter username
                                error("User name is taken");
                            // } else if (strstr(temp_str, SERVER_USER_APPROVE) != NULL) {
                            } else if(startsWith(temp_str, SERVER_USER_APPROVE)) {
                                login_state = 4; // username approved
                            } else if (strlen(temp_str) == 0) {
                                // printf_error(stderr, "\n==>%s<<\n", temp_str);
                                break;
                            } else {
                                // printf_error(stderr, "\n==>%s<<\n", temp_str);
                                error("Invalid systex on username approval notice");
                            }
                            // if(strcmp(temp_str, SERVER_USER_TAKEN) == 0) {
                            //     login_state = 5; // re enter username
                            //     // error("Expected server hello, got something else");
                            // } else if (strcmp(temp_str, SERVER_USER_APPROVE) == 0) {
                            //     login_state = 4; // username approved
                            // } else {
                            //     error("Invalid systex on username approval notice");
                            // }
                        }
                    }
                    break;
                case 4: // server msg
                    if(FD_ISSET(sockfd, &rset)) { // socket is readable
                        if((n = read(sockfd, buf, MAXLINE)) == 0) {
                            error("Server exited prevaturely before MOTD");
                        }
                        temp_str = get_string_without_ending(buf);
                        if (strlen(temp_str) == 0) {
                            // printf_error(stderr, "\n==>%s<<\n", temp_str);
                            break;
                        // } else if(strstr(temp_str, SERVER_MOTD) == NULL) {
                        } else if(!startsWith(temp_str, SERVER_MOTD)) {
                            error("Invalid systex on MOTD");
                        }
                        write(fileno(stdout), temp_str + strlen(SERVER_MOTD) + 1, n - strlen(SERVER_MOTD) + 1);
                        login_state = 10;
                    }
                    break;
            }
            // if(FD_ISSET(sockfd, &rset)) { // socket is readable
            //     if((n = read(sockfd, buf, MAXLINE)) == 0) {
            //         if(stdineof == 1) {
            //             return;
            //         } else {
            //             error("login_proc: server terminated prematurely");
            //         }
            //     }
            //     /*this is where I will have to do the protocol stuff. I think lol*/
            //     write(fileno(stdout), buf, n);
            // }
            // if(FD_ISSET(fileno(fp), &rset)) { // input is readable
            //     if((n = read(fileno(fp), buf, MAXLINE)) == 0) {
            //         stdineof = 1;
            //         shutdown(sockfd, SHUT_WR); // send fin
            //         FD_CLR(fileno(fp), &rset);
            //         continue;
            //     }
            //     write(sockfd, buf, n);
            // }
        }
    }
}

void regular_chat(FILE *fp, int sockfd) {
    int maxfdpl, stdineof, n;
    fd_set rset;
    char buf[MAXLINE];

    stdineof = 0;
    FD_ZERO(&rset);
    struct timeval timeout;

    while(1) {
        memset(buf, 0, MAXLINE);
        if(stdineof == 0) {
            FD_SET(fileno(fp), &rset);
        }
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        FD_SET(sockfd, &rset);
        maxfdpl = MAX(fileno(fp), sockfd) + 1;
        if(select(maxfdpl, &rset, NULL, NULL, &timeout) != 0) {
            if(FD_ISSET(sockfd, &rset)) { // socket is readable
                if((n = read(sockfd, buf, MAXLINE)) == 0) {
                    if(stdineof == 1) {
                        return;
                    } else {
                        error("chat procedure: server terminated prematurely");
                    }
                }
                /*this is where I will have to do the protocol stuff. I think lol*/
                if(strlen(buf) == 0 || startsWith(buf, "\r\n") ||startsWith(buf, "\r\n\r\n")) {
                    continue;
                }
                if(startsWith(buf, SERVER_LIST_USER)) {
                    char *user_ptr = strtok(buf + strlen(SERVER_LIST_USER), " ");
                    while(user_ptr != NULL) {
                        write(fileno(stdout), user_ptr, strlen(user_ptr));
                        write(fileno(stdout), "\n", 1);
                        user_ptr = strtok(NULL, " ");
                    }
                    continue;
                } else if (startsWith(buf, CONFIRM_MSG_TO)) {
                    char *tmp_msg = "\x1B[1;33m MSG sent successfully.\x1B[0m\n";
                    write(fileno(stdout), tmp_msg, strlen(tmp_msg));
                    // if it is the first time I should be establishing connection and increasing the counter on the msges to keep track. I also should ckeck if the msg is correctly sent, or just garbage.
                    continue;
                } else if (startsWith(buf, MSG_FROM)) {
                    char *buf_cpy = calloc(strlen(buf), 1);
                    memcpy(buf_cpy, buf, strlen(buf));
                    char *msg_user = strtok(buf_cpy + strlen(MSG_FROM) + 1, " ");
                    write(fileno(stdout), buf + strlen(MSG_FROM), strlen(buf + strlen(MSG_FROM)));
                    // TODO create specific functions for each of these specific actons. So I can change them later faster than now!
                    char *msg_to_sent = calloc(strlen(CONFIRM_MSG_FROM) + strlen(msg_user) + 5, sizeof(char));
                    memcpy(msg_to_sent, CONFIRM_MSG_FROM, strlen(CONFIRM_MSG_FROM));
                    memcpy(msg_to_sent + strlen(msg_to_sent), " ", 1);
                    memcpy(msg_to_sent + strlen(msg_to_sent), msg_user, strlen(msg_user));
                    memcpy(msg_to_sent + strlen(msg_to_sent), "\r\n\r\n", 4);
                    write(sockfd, msg_to_sent, strlen(msg_to_sent));
                    continue;
                } else if (startsWith(buf, NO_USR)) {
                    char *no_usr = "No, user = ";
                    char *no_usr_exists = " exists.\n";
                    write(fileno(stdout), no_usr, strlen(no_usr));
                    write(fileno(stdout), buf + strlen(NO_USR), strlen(buf + strlen(NO_USR)) - 4);
                    write(fileno(stdout), no_usr_exists, strlen(no_usr_exists));
                    continue;
                } else if (startsWith(buf, SERVER_BYE_ACK)) {
                    return;
                } else if (startsWith(buf, SERVER_BYE_BROD)) {
                    char *client_closed = "Client closed with the name of = ";
                    write(fileno(stdout), client_closed, strlen(client_closed));
                    write(fileno(stdout), buf + strlen(SERVER_BYE_BROD), strlen(buf + strlen(SERVER_BYE_BROD)) - 4);
                    continue;
                } else {
                    printf_error(stderr, "Server sent >>%s<<\n", buf);
                    error("Server sent msg that cannot be understood");
                }
                // write(fileno(stdout), buf, n);
            }
            if(FD_ISSET(fileno(fp), &rset)) { // input is readable
                if((n = read(fileno(fp), buf, MAXLINE)) == 0) {
                    stdineof = 1;
                    shutdown(sockfd, SHUT_WR); // send fin
                    FD_CLR(fileno(fp), &rset);
                    continue;
                }
                // int what_to_do = -1;
                if(strlen(buf) == 0) {
                    continue;
                }
                if(*buf != '/') {
                    printf_error(stderr, "You did not use the right format! Type /help to see suggestions\n");
                    continue;
                }
                if(startsWith(buf, USER_HELP)) {
                    print_client_help();
                    continue;
                } else if(startsWith(buf, USER_LISTU)) {
                    // TODO I will come back to fix this later
                    write(sockfd, set_string_ending(CLIENT_LIST_USER), strlen(CLIENT_LIST_USER) + 4);
                    continue;
                } else if(startsWith(buf, USER_CHAT)) {
                    char *new_buf = buf + strlen(USER_CHAT);
                    char *new_str = calloc(strlen(MSG_TO) + strlen(new_buf) + 4, sizeof(char));
                    memcpy(new_str, MSG_TO, strlen(MSG_TO));
                    memcpy(new_str + strlen(new_str), new_buf, strlen(new_buf));
                    memcpy(new_str + strlen(new_str), "\r\n\r\n", 4);
                    write(sockfd, new_str, strlen(new_str));
                    // This later on down the line will use the list to open new windows and keep track of the users
                    continue;
                } else if(startsWith(buf, USER_LOGOUT)) {
                    write(sockfd, set_string_ending(CLIENT_BYE), strlen(CLIENT_BYE) + 4);
                    stdineof = 1;
                    shutdown(sockfd, SHUT_WR); // send fin
                    FD_CLR(fileno(fp), &rset);
                    continue;
                } else {
                    stdineof = 1;
                    shutdown(sockfd, SHUT_WR); // send fin
                    FD_CLR(fileno(fp), &rset);
                    error("Client sent msg that cannot be understood");
                }
                // write(sockfd, buf, n);
            }
        }
    }
}

void print_client_help() {
    char *help_string = "\n\x1B[1;34m You can say the following commands\n\
    /help               To see this help menu\n\
    /logout             To logout of the chat. This also closes all windows\n\
    /listu              Request a list of logged in users\n\
    /chat <to> <msg>    requires username of logged in user, and the msg you want to send\x1B[0m\n";
    write(fileno(stdout), help_string, strlen(help_string));
}

bool startsWith(char *str, char *pre) {
    size_t len_str = strlen(str);
    size_t len_pre = strlen(pre);

    return len_str < len_pre ? false : strncmp(str, pre, len_pre) == 0;
}

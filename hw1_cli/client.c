#include "client.h"

char CURRENT_USER_NAME[20];
char read_buf[MAXLINE];
char *full_buf;
fd_set rset;

int max_size_of_full_buf = 0, maxfdpl = 0;

int main(int argc, char **argv) {
    signal(SIGCHLD, sigchild_handler);

    user_list_head = NULL;
    user_list_tail = NULL;

    int sockfd = 0, portno;
    struct sockaddr_in servaddr;
    char* hostaddr;
    full_buf = NULL;
    increase_full_buf(full_buf);
    // check if the minimum argv req is met
    if (argc < 4) {
        print_usage(EXIT_FAILURE);
    }
    int count = 1;
    if(argv[1][0] == '-') {
        if(argv[1][1] == 'h') {
            print_usage(EXIT_SUCCESS);
        } else if (argv[1][1] == 'v') {
            print_verbose();
        } else {
            print_usage(EXIT_FAILURE);
        }
        count++;
    }

	memcpy(CURRENT_USER_NAME, argv[count], MIN(20, strlen(argv[count])));
	hostaddr = argv[count + 1];
	portno = atoi(argv[count + 2]);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error("scoket error");
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(portno);

	if(inet_pton(AF_INET, hostaddr, &servaddr.sin_addr) <= 0) {
		error("inet_pton error");
	}

    if(connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        error("connect error");
    }

    // login procedure
    login_proc(stdin, sockfd);

    regular_chat(stdin, sockfd);

    close(sockfd);
    proper_exit();

}

void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void proper_exit() {
    printf("proper exit\n");
    free(full_buf);
    exit(EXIT_SUCCESS);
}

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

void login_proc(FILE *fp, int sockfd) {
    int maxfdpl, stdineof = 0, n, login_state = 0;
    fd_set rset;
    struct timeval timeout;

    FD_ZERO(&rset);

    while(login_state != 10) {
        memset(read_buf, 0, MAXLINE);
        if(stdineof == 0) {
            FD_SET(fileno(fp), &rset);
        }
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        FD_SET(sockfd, &rset);
        maxfdpl = MAX(fileno(fp), sockfd) + 1;
        if(login_state == 0) {
            send_login_msg(sockfd);
            login_state = 1;
        }
        if(select(maxfdpl, &rset, NULL, NULL, &timeout) != 0) {
            switch(login_state) {
                case 1:
                    if(FD_ISSET(sockfd, &rset)) {
                        if((n = read(sockfd, read_buf, MAXLINE)) == 0) {
                            error("Server exited prematurely while in hello process");
                        }
                        char *process_str = process_string(read_buf);
                        if(process_str != NULL) {
                            if(strcmp(process_str, SERVER_LOGIN_INIT) != 0) {
                                error("Expected Server Hello, got something else");
                            }
                            login_state = 2;
                            free(process_str);
                        } // else login state stays as 1, and keep looking for server hello
                    }
                    if(login_state != 2) {
                        break;
                    }
                case 2:
                    send_username(sockfd);
                    login_state = 3;
                    break;
                case 3:
                    if(FD_ISSET(sockfd, &rset)) {
                        if((n = read(sockfd, read_buf, MAXLINE)) == 0) {
                            error("Server exited prematurely while username approval is in proces");
                        }
                        char *process_str = process_string(read_buf);
                        if(process_str != NULL) {
                            if(strcmp(process_str, SERVER_USER_TAKEN) == 0) {
                                write(fileno(stdout), USERNAME_TAKEN, strlen(USERNAME_TAKEN));
                                // error(USERNAME_TAKEN);
                                exit(EXIT_FAILURE);
                            } else if(strcmp(process_str, SERVER_USER_APPROVE) == 0) {
                                free(process_str);
                                login_state = 4;
                            } else {
                                error("Invalid syntex on username approval notice");
                            }
                        }
                    }
                    if(login_state != 4) {
                        break;
                    }
                case 4:
                    if(FD_ISSET(sockfd, &rset)) {
                        if((n = read(sockfd, read_buf, MAXLINE)) == 0) {
                            error("Server exited prevaturely before MOTD");
                        }
                        char *process_str = process_string(read_buf);
                        if(process_str != NULL) {
                            if(strncmp(process_str, SERVER_MOTD, 4) != 0) {
                                error("Invalid systex on MOTD");
                            }
                            write(fileno(stdout), process_str + strlen(SERVER_MOTD) + 1, n - strlen(SERVER_MOTD) + 1);
                            login_state = 10;
                            free(process_str);
                        }
                    }
                    break;
            }
        }
    }
}

void set_fd_for_childs(fd_set rset) {
    list_user *temp = user_list_head;
    while(temp != NULL) {
        if(temp->curr_user != NULL) {
            if(temp->curr_user->active) {
                FD_SET(temp->curr_user->rd, &rset);
                FD_SET(temp->curr_user->wrt, &rset);
                maxfdpl = MAX(maxfdpl, temp->curr_user->rd) + 1;
                maxfdpl = MAX(maxfdpl, temp->curr_user->wrt) + 1;
            }
        }
        temp = temp->next;
    }
}

void regular_chat(FILE *fp, int sockfd) {
    int stdineof = 0, n;
    struct timeval timeout;

    FD_ZERO(&rset);

    maxfdpl = MAX(fileno(fp), sockfd) + 1;

    while(1) {
        memset(read_buf, 0, MAXLINE);
        if(stdineof == 0) {
            FD_SET(fileno(fp), &rset);
        }
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        FD_SET(sockfd, &rset);
        set_fd_for_childs(rset);
        if(select(maxfdpl, &rset, NULL, NULL, &timeout) != 0) {
            if(FD_ISSET(sockfd, &rset)) {
                if((n = read(sockfd, read_buf, MAXLINE)) == 0) {
                    if(stdineof == 1) {
                        return;
                    } else {
                        error("chat procedure: server terminated prematurely");
                    }
                }
                char *process_str = process_string(read_buf);
                if(process_str != NULL) { // if it is null then more input is coming
                    int first_wd_l = word_length(process_str);
                    if(strncmp(process_str, SERVER_LIST_USER, first_wd_l) == 0) {
                        list_users(process_str + first_wd_l + 1);
                        free(process_str);
                        continue;
                    } else if (strncmp(process_str, CONFIRM_MSG_TO, first_wd_l) == 0) {
                        msg_confirmed(process_str + first_wd_l  + 1);
                        free(process_str);
                        continue;
                    } else if (strncmp(process_str, NO_USR, first_wd_l) == 0) {
                        no_user_exists(process_str + first_wd_l + 1);
                        free(process_str);
                        continue;
                    } else if (strncmp(process_str, MSG_FROM, first_wd_l) == 0) {
                        process_msg(process_str + first_wd_l + 1);
                        char temp[40] = {};
                        strcat(temp, CONFIRM_MSG_FROM);
                        strcat(temp, " ");
                        strcat(temp, process_str + first_wd_l  + 1);
                        strcat(temp, ENDING);
                        write(sockfd, temp, strlen(temp));
                        continue;
                    } else if (strncmp(process_str, SERVER_BYE_BROD, first_wd_l) == 0) {
                        someone_exited(process_str + first_wd_l + 1);
                        free(process_str);
                        continue;
                    } else if (strncmp(process_str, SERVER_BYE_ACK, first_wd_l) == 0) {
                        free(process_str);
                        return;
                    }
                }
            }
        }
        if(FD_ISSET(fileno(fp), &rset)) {
            if((n = read(fileno(fp), read_buf, MAXLINE)) == 0) {
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            if(*read_buf != '/') {
                continue;
            }
            int first_wd_l = word_length(read_buf);
            if(strncmp(read_buf, USER_HELP, first_wd_l) == 0) {
                print_client_help();
                continue;
            }
            
            if(strncmp(read_buf, USER_LISTU, first_wd_l) == 0) {
                char *temp = set_string_ending(CLIENT_LIST_USER);
                write(sockfd, temp, strlen(temp));
                free(temp);
                continue;
            }

            if(strncmp(read_buf, USER_CHAT, first_wd_l) == 0) {
                char *new_buf = read_buf + strlen(USER_CHAT);
                char *new_str = calloc(strlen(MSG_TO) + strlen(new_buf) + 4, sizeof(char));
                if(new_str == NULL) {
                    error("Error on calloc");
                }
                process_msg(new_buf + 1);
                memcpy(new_str, MSG_TO, strlen(MSG_TO));
                memcpy(new_str + strlen(new_str), new_buf, strlen(new_buf));
                memcpy(new_str + strlen(new_str), "\r\n\r\n", 4);
                write(sockfd, new_str, strlen(new_str));
                free(new_str);
                continue;
            }
            if(strncmp(read_buf, USER_LOGOUT, first_wd_l) == 0) {
                char *temp = set_string_ending(CLIENT_BYE);
                write(sockfd, temp, strlen(temp));
                stdineof = 1;
                shutdown(sockfd, SHUT_WR); // send fin
                FD_CLR(fileno(fp), &rset);
                free(temp);
                continue;
            }

            stdineof = 1;
            shutdown(sockfd, SHUT_WR);
            FD_CLR(fileno(fp), &rset);
            error("Client sent msg that cannot be understood");
        }
        any_children_replied(rset, sockfd);
    }

}

void any_children_replied(fd_set rset, int sockfd) {
    list_user *temp = user_list_head;
    int n = 0;
    while(temp != NULL) {
        if(temp->curr_user != NULL) {
            if(temp->curr_user->active) {
                if(FD_ISSET(temp->curr_user->rd, &rset)) {
                    if((n = read(temp->curr_user->rd, read_buf, MAXLINE)) == 0) {
                        temp->curr_user->active = false;
                        continue;
                    }
                    char *new_buf = read_buf;
                    char *new_str = calloc(strlen(MSG_TO) + strlen(new_buf) + 24, sizeof(char));
                    if(new_str == NULL) {
                        error("Error on calloc");
                    }
                    memcpy(new_str, MSG_TO, strlen(MSG_TO));
                    memcpy(new_str + strlen(new_str), temp->curr_user->username, strlen(temp->curr_user->username));
                    memcpy(new_str + strlen(new_str), " ", 1);
                    memcpy(new_str + strlen(new_str), new_buf, strlen(new_buf));
                    memcpy(new_str + strlen(new_str), "\r\n\r\n", 4);

                    write(fileno(stdout), new_str, strlen(new_str));
                    
                    write(sockfd, new_str, strlen(new_str));
                    free(new_str);
                } else if(FD_ISSET(temp->curr_user->wrt, &rset)) {
                    if((n = read(temp->curr_user->rd, read_buf, MAXLINE)) == 0) {
                        temp->curr_user->active = false;
                        continue;
                    }
                    char *new_buf = read_buf;
                    char *new_str = calloc(strlen(MSG_TO) + strlen(new_buf) + 24, sizeof(char));
                    if(new_str == NULL) {
                        error("Error on calloc");
                    }
                    memcpy(new_str, MSG_TO, strlen(MSG_TO));
                    memcpy(new_str + strlen(new_str), temp->curr_user->username, strlen(temp->curr_user->username));
                    memcpy(new_str + strlen(new_str), " ", 1);
                    memcpy(new_str + strlen(new_str), new_buf, strlen(new_buf));
                    memcpy(new_str + strlen(new_str), "\r\n\r\n", 4);

                    write(fileno(stdout), new_str, strlen(new_str));
                    
                    write(sockfd, new_str, strlen(new_str));
                    free(new_str);
                }
            }
        }
        temp = temp->next;
    }
}
char* set_string_ending(char* str) {
    size_t old_size = strlen(str);
    char* to_ret = calloc(old_size, sizeof(char));
    memcpy(to_ret, str, old_size);
    memcpy(to_ret + old_size, ENDING, 4);
    return to_ret;
}

void print_client_help() {
    char *help_string = "\n\x1B[1;34m You can say the following commands\n\
    /help               To see this help menu\n\
    /logout             To logout of the chat. This also closes all windows\n\
    /listu              Request a list of logged in users\n\
    /chat <to> <msg>    requires username of logged in user, and the msg you want to send\x1B[0m\n";
    write(fileno(stdout), help_string, strlen(help_string));
}


int word_length(char *string) {
    char ch = *string;
    int i = 0;
    while(ch != '\0' && ch != '\t' && ch != ' ' && ch != '\n') {
        i++;
        ch = *(string + i);
    }
    return i;
}

void list_users(char *user_list) {
    char *user_ptr = strtok(user_list, " ");
    while(user_ptr != NULL) {
        write(fileno(stdout), user_ptr, strlen(user_ptr));
        write(fileno(stdout), "\n", 1);
        user_ptr = strtok(NULL, " ");
    }
}

void no_user_exists(char *string) {
    char *no_usr = "No, user = ";
    char *no_usr_exists = " exists.\n";
    write(fileno(stdout), no_usr, strlen(no_usr));
    write(fileno(stdout), string, strlen(string));
    write(fileno(stdout), no_usr_exists, strlen(no_usr_exists));
}

void someone_exited(char *string) {
    char *client_closed = "Client closed with the name of = ";
    write(fileno(stdout), client_closed, strlen(client_closed));
    write(fileno(stdout), string, strlen(string));
}

void send_username(int sockfd) {
    char *temp_str = calloc(30, 1);
    if(temp_str == NULL) {
        error("send_username calloc was null");
    }
    memcpy(temp_str, CLIENT_SET_USER, 3);
    memcpy(temp_str + 3, " ", 1);
    memcpy(temp_str + 4, CURRENT_USER_NAME, strlen(CURRENT_USER_NAME));
    strcat(temp_str, ENDING);
    write(sockfd, temp_str, strlen(temp_str));
    free(temp_str);
}

void msg_confirmed(char *name) {
    user_struct *temp_user = find_user(name);
    if(temp_user == NULL) {
        error("Got msg confirm msg for a user that never existed");
    }
    temp_user->ot_cnt++;
    // if(temp_user->ot_cnt > temp_user->to_cnt) {
    //     error("Got msg confirm msg more than msg sent");
    // }
}

void process_msg(char *msg) {
    int name_len = word_length(msg);
    char name[name_len + 1];
    memset(name, 0, name_len + 1);
    memcpy(name, msg, name_len);
    user_struct *temp_user = find_user(name);
    if(temp_user == NULL) {
        temp_user = create_new_user(name);
    }
    write(temp_user->wrt, msg + name_len + 1, strlen(msg + name_len + 1));
}

user_struct* find_user(char *name) {
    list_user *temp = user_list_head;
    while(temp != NULL) {
        if(temp->curr_user != NULL) {
            if(strcmp(temp->curr_user->username, name) == 0) {
                return temp->curr_user;
            }
        }
        temp = temp->next;
    }
    return NULL;
}

user_struct* create_new_user(char *name) {
    user_struct *new_usr = calloc(1, sizeof(user_struct));
    new_usr->username = calloc(strlen(name), 1);
    memcpy(new_usr->username, name, strlen(name));

    list_user *temp_list = calloc(1, sizeof(list_user));
    temp_list->curr_user = new_usr;
    if(user_list_head == NULL) {
        user_list_head = temp_list;
        user_list_tail = user_list_head;
    } else {
        user_list_tail->next = temp_list;
        temp_list->prev = user_list_tail;
        user_list_tail = temp_list;
    }
    

    int fd[2];
    pid_t childpid;
    pipe(fd);
    perror("right b4 fork");
    if((childpid = fork()) == -1) {
        error("Error while forking to start xterm");
    }
    if(childpid == 0) {
        // close(fd[1]);
        char rd[10];
        char wrt[10];
        sprintf(rd, "%d", fd[0]);
        sprintf(wrt, "%d", fd[1]);
        char *args[8] = { "xterm", "-T", new_usr->username , "-e", "./chat", rd, wrt , NULL };
        if(execvp("xterm", args) == -1) {
            error("could not execute xterm");
        }
    } else {
        new_usr->rd = fd[0];
        new_usr->wrt = fd[1];
        new_usr->active = true;
        temp_list->pid = childpid;
        set_fd_for_childs(rset);
        return new_usr;
    }

    return NULL;
}

void increase_full_buf() {
    int temp = max_size_of_full_buf;
    max_size_of_full_buf += MAXLINE;
    if(full_buf == NULL) {
        full_buf = malloc(MAXLINE);
    } else {
        full_buf = realloc(full_buf, max_size_of_full_buf);
    }
    if(full_buf == NULL) {
        error("Error while realloc");
    }
    memset(full_buf + temp, 0, MAXLINE);
}
void reset_full_buf() {
    free(full_buf);
    full_buf = NULL;
    max_size_of_full_buf = 0;
    increase_full_buf();
}
void send_login_msg(int sockfd) {
    char *msg_to_sent = "ME2U\r\n\r\n";
    write(sockfd, msg_to_sent, strlen(msg_to_sent));
}

char* process_string(char * buf) {
    char *to_ret = NULL;
    int size = strlen(buf), i = 0;
    if(strlen(full_buf) + size > max_size_of_full_buf - 1) {
        increase_full_buf();
    }
    strcat(full_buf, buf);
    if(strncmp(full_buf + strlen(full_buf) - 4, ENDING, 4) == 0) {
        to_ret = calloc(strlen(full_buf), 1);
        if(to_ret == NULL) {
            error("Error while callocking process_string");
        }
        memcpy(to_ret, full_buf, strlen(full_buf) - 4);
        reset_full_buf();
    }
    return to_ret;
}

void color_verbose() {
    write(fileno(stdout), C_VERBOSE, strlen(C_VERBOSE));
}

void color_reset() {
    write(fileno(stdout), C_DEFAULT, strlen(C_DEFAULT));
}

void sigchild_handler(int sig) {
    pid_t pid;
    int status;
    while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        list_user *temp = find_user_pid(pid);
        if(temp->curr_user == NULL) {
            error("PID found but user is null");
        }
        temp->curr_user->active = false;
    }
}

list_user* find_user_pid(pid_t pid) {
    list_user *temp = user_list_head;
    while(temp != NULL) {
        if(temp->pid == pid) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}
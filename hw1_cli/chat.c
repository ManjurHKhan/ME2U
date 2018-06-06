#include "chat.h"

int rd = 0;
int wrt = 0;

int main(int argc, char **argv) {
    if(argc < 2) {
        error("Invalid args. ./chat fd");
    }
    rd = atoi(argv[1]);
    wrt = atoi(argv[2]);
    
    FILE *fp = stdin;

    int maxfdpl, stdineof = 0, n;
    fd_set rset;
    char buf[MAXLINE];

    FD_ZERO(&rset);
    struct timeval timeout;

    while(1) {
        memset(buf, 0, MAXLINE);
        if(stdineof == 0) {
            FD_SET(fileno(fp), &rset);
        }
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        FD_SET(rd, &rset);
        FD_SET(wrt, &rset);

        maxfdpl = MAX(fileno(fp), rd) + 1;
        if(select(maxfdpl, &rset, NULL, NULL, &timeout) != 0) {
            if(FD_ISSET(rd, &rset)) {
                if((n = read(rd, buf, MAXLINE)) == 0) {
                    break;
                }
                write_to_screen(fileno(stdout), buf, "> ", RECV_MSG_C);
            } else if(FD_ISSET(fileno(fp), &rset)) {
                if((n = read(fileno(fp), buf, MAXLINE)) == 0) {
                    break;
                }
                write_to_screen(fileno(stdout), buf, "< ", SENT_MSG_C);

                write(rd, buf, strlen(buf));
            }
        }
    }

    exit(EXIT_SUCCESS);
}

void error(char *msg) {
    perror(msg);
    exit(0);
}

void write_to_screen(int fd, char *buf, char *arrow, char *color) {
    write(fd, color, strlen(color));
    write(fd, arrow, strlen(arrow));
    write(fd, buf, strlen(buf));
    write(fd, DEFAULT_C, strlen(DEFAULT_C));
}
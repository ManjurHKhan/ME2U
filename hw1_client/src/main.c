#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "client.h"
#include "debug.h"

int main(int argc, char **argv) {
    // char CURRENT_USER_NAME[20];
    int sockfd = 0, portno;
    struct sockaddr_in servaddr;
    char* hostaddr;
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
    // print_info("%d\n", count);
    // print_info("argv[1] = %s, argv[2] = %s, argv[3] = %s\n", argv[1], argv[2], argv[3]);
    // CURRENT_USER_NAME = argv[count];
    memcpy(CURRENT_USER_NAME, argv[count], MIN(20, strlen(argv[count])));
    // printf_error(stderr, "user name is =>> %s\n", CURRENT_USER_NAME);
    // strcpy(CURRENT_USER_NAME, argv[count]);
    // while(!valid_username(CURRENT_USER_NAME)) {
    //     print_error("Invalid username. Must be under 10 character and cannot contain spcaes");
    //     bzero(CURRENT_USER_NAME, 20);
    //     scanf("Enter Username: %[^\n]s", CURRENT_USER_NAME);
    // }
    hostaddr = argv[count + 1];
    portno = atoi(argv[count + 2]);
    // print_info("I am here sockfd = %d\n", sockfd);
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("socket error");
    }
    // print_info("socket fd[%d] was found\n", sockfd);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portno);
    // print_info("%s\n", hostaddr);
    // print_info("=>> %x\n", servaddr.sin_addr);
    if(inet_pton(AF_INET, hostaddr, &servaddr.sin_addr) <= 0) {
        error("inet_pton error");
    }
    // print_info("==>> %x\n", servaddr.sin_addr);
    // print_info("inet_pron didnt give error\n");
    if(connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        error("connect error");
    }
    login_proc(stdin, sockfd);
    regular_chat(stdin, sockfd);
    // printf("%s\n", "life is hard");
    exit(EXIT_SUCCESS);
}

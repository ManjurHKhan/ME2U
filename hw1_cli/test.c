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

int main() {
    char *args[5] = { "xterm", "-T", "name" , "-e", "python"};
    execve("xterm", args, NULL);
}
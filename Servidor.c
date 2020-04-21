#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define LISTENQ 10
#define MAXDATASIZE 100
#define SERV_PORT 3490

int main(int argc, char** argv) {
    int    listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in servaddr, cliaddr;
    char   buf[MAXDATASIZE];
    pid_t childpid;
    time_t ticks;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        exit(1);
    }

    for (; ; ) {
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen)) == -1) {
            perror("accept");
            exit(1);
        }

        if ((childpid = fork()) == 0) {
            close(listenfd);
            // TODO: operações no catálogo de filmes
            exit(0);
        }

        close(connfd);
    }
    return(0);
}
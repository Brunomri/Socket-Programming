#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SERV_PORT 3490

// socket()
// connect()
// write()
// read()
// close()

int main (int argc, char** argv) {
	int sockfd;
	struct sockaddr_in servaddr;

	if (argc != 2) {
		perror("usage: tcpcli <IPaddress>");
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket error");
		exit(1);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
		perror("inet_pton error");
		exit(1);
	}

	if (connect(sockfd, (struct sockaddr*) & servaddr, sizeof(servaddr)) < 0) {
		perror("connect error");
		exit(1);
	}

	// TODO: operações no catálogo de filmes

	exit(0);
}
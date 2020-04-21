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
#define MAXDATASIZE 100

void func(int sockfd)
{
	char buff[MAXDATASIZE];
	int n;
	for (;;) {
		bzero(buff, sizeof(buff));
		printf("Enter the string : ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n');
		write(sockfd, buff, sizeof(buff));
		bzero(buff, sizeof(buff));
		read(sockfd, buff, sizeof(buff));
		printf("From Server : %s", buff);
		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}
	}
}

// Estrutura filme com título, sinopse, gênero, salas em exibição e identificador único
typedef struct filmes
{
	char* id;
	char* titulo;
	char* sinopse;
	char* genero;
	char* salas;
} filme;

void cadastrar(int sockfd) {
	filme novo = {NULL, "O Irlandes", "Filme sobre mafia", "Acao", "1,2,3"};
	char* buff = novo.titulo;
	printf("Enviando filme: %s\n", buff);
	write(sockfd, buff, sizeof(buff));
}

int main (int argc, char** argv) {
	int sockfd;
	char buf[MAXDATASIZE];
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

	printf("Conexao estabelecida\n");

	// TODO: operações no catálogo de filmes

	//func(sockfd);
	cadastrar(sockfd);

	exit(0);
}
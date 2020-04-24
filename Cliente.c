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
#define MAXDATASIZE 500

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
//typedef struct filmes
//{
//	char id[MAXDATASIZE];
//	char titulo[MAXDATASIZE];
//	char sinopse[MAXDATASIZE];
//	char genero[MAXDATASIZE];
//	char salas[MAXDATASIZE];
//} filme;

/*
 * Funcao:  readn
 * --------------
 * Le n bytes de um socket. Repete o procedimento caso
 * limite do buffer seja atingido
 *
 * fd: inteiro descritor do socket
 * vptr: ponteiro para variavel que armazena os dados
 * n: numero de bytes a serem lidos
 *
 * retorna: numero de bytes lidos
 */
ssize_t readn(int fd, void* vptr, size_t n) {
	size_t nleft;
	ssize_t nread;
	char* ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;
			else
				return -1;
		}
		else if (nread == 0)
			break;
		nleft -= nread;
		ptr += nread;
	}
	return (n - nleft);
}

/*
 * Funcao:  writen
 * ---------------
 * Escreve n bytes em um socket. Repete o procedimento caso
 * limite do buffer seja atingido
 *
 * fd: inteiro descritor do socket
 * vptr: ponteiro para variavel que armazena os dados
 * n: numero de bytes a serem escritos
 *
 * retorna: numero de bytes escritos
 */
ssize_t writen(int fd, const void* vptr, size_t n) {
	size_t nleft;
	ssize_t nwritten;
	const char* ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else
				return -1;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}

/*
 * Funcao:  enviar
 * ---------------
 * Encapsula 2 chamadas de writen para enviar o tamanho da variavel
 * antes do envio dos dados
 *
 * sockfd: inteiro descritor do socket
 * buff: ponteiro para variavel que armazena os dados
 * size: numero de bytes a enviar
 *
 */
void enviar(int sockfd, const void* buff, size_t size) {
	writen(sockfd, &size, sizeof(size_t));
	writen(sockfd, buff, size);
	printf("Enviando: %s (%d bytes)\n", buff, size);
}

/*
 * Funcao:  receber
 * ----------------
 * Encapsula 2 chamadas de readn para receber o tamanho da variavel
 * antes da leitura dos dados
 *
 * sockfd: inteiro descritor do socket
 * buff: ponteiro para variavel que armazena os dados
 *
 */
void receber(int sockfd, void* buff) {
	size_t size;
	readn(sockfd, &size, sizeof(size_t));
	readn(sockfd, buff, size);
	printf("Recebendo: %s (%d bytes)\n", buff, size);
}

/*
 * Funcao:  cadastrar
 * ------------------
 * Cadastra um novo filme enviando ao servidor seu titulo, sinopse, genero e
 * salas, recebendo do servidor um identificador unico
 *
 * sockfd: inteiro descritor do socket
 *
 */
void cadastrar(int sockfd) {
	char titulo[] = "1917";
	enviar(sockfd, titulo, sizeof(titulo));

	char sinopse[] = "Um filme sobre guerra";
	enviar(sockfd, sinopse, sizeof(sinopse));

	char genero[] = "acao";
	enviar(sockfd, genero, sizeof(genero));

	char salas[] = "1,2,3";
	enviar(sockfd, salas, sizeof(salas));

	char id[MAXDATASIZE];
	receber(sockfd, id);

	printf("\nFilme cadastrado com sucesso, id: %s\n", id);
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
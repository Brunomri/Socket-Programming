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
 * Funcao: readn
 * -------------
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
 * Funcao: writen
 * --------------
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
 * Funcao: enviar
 * --------------
 * Encapsula 2 chamadas de writen para enviar o tamanho da variavel
 * antes do envio dos dados
 *
 * sockfd: inteiro descritor do socket
 * buff: ponteiro para variavel que armazena os dados
 * size: numero de bytes a enviar
 *
 */
void enviar(int sockfd, const char* buff, size_t size) {
	writen(sockfd, &size, sizeof(size_t));
	writen(sockfd, buff, size);
	printf("\nEnviando: %s (%d bytes)\n", buff, size);
}

/*
 * Funcao: receber
 * ---------------
 * Encapsula 2 chamadas de readn para receber o tamanho da variavel
 * antes da leitura dos dados
 *
 * sockfd: inteiro descritor do socket
 *
 * retorna: ponteiro para os dados recebidos
 */
char* receber(int sockfd) {
	size_t size;
	readn(sockfd, &size, sizeof(size_t));
	char* buff = (void*)malloc(size * sizeof(char));
	readn(sockfd, buff, size);
	printf("\nRecebendo: %s (%d bytes)\n", buff, size);
	return buff;
}

/*
 * Funcao: lerConsole
 * ------------------
 * Faz a leitura do console em um caractere por vez para alocar
 * dinamicamente a memoria necessaria para as variaveis
 *
 */
char* lerConsole() {
	char* line = NULL, * tmp = NULL;
	size_t size = 0, index = 0;
	int ch = EOF;

	while (ch) {
		ch = getc(stdin);

		/* Verifica se cadeia chegou ao fim */
		if (ch == EOF || ch == '\n')
			ch = 0;

		/* Verifica se precisa alocar mais memoria */
		if (size <= index) {
			size += 1;
			tmp = realloc(line, size);
			if (!tmp) {
				free(line);
				line = NULL;
				break;
			}
			line = tmp;
		}

		/* Armazena conteudo */
		line[index++] = ch;
	}

	return line;
}

/*
 * Funcao: cadastrar
 * -----------------
 * Cadastra um novo filme enviando ao servidor seu titulo, sinopse, genero e
 * salas, recebendo do servidor um identificador unico
 *
 * sockfd: inteiro descritor do socket
 *
 */
void cadastrar(int sockfd) {
	printf("\nCadastrar novo filme:\n");
	printf("\nInsira o titulo:\n");
	char* titulo = lerConsole();
	
	printf("\nInsira a sinopse:\n");
	char* sinopse = lerConsole();
	
	printf("\nInsira o genero:\n");
	char* genero = lerConsole();
	
	printf("\nInsira as salas:\n");
	char* salas = lerConsole();

	enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char));
	enviar(sockfd, sinopse, (strlen(sinopse) + 1) * sizeof(char));
	enviar(sockfd, genero, (strlen(genero) + 1) * sizeof(char));
	enviar(sockfd, salas, (strlen(salas) + 1) * sizeof(char));

	char *id = receber(sockfd);

	printf("\nNovo filme cadastrado:\n");
	printf("Id: %s\n", id);
	printf("Titulo: %s\n", titulo);
	printf("Sinopse: %s\n", sinopse);
	printf("Genero: %s\n", genero);
	printf("Salas: %s\n", salas);

	free(titulo);
	free(sinopse);
	free(genero);
	free(salas);
	free(id);
}

/*
 * Funcao: remover
 * ---------------
 * Remove um filme existente a partir do seu identificador
 *
 * sockfd: inteiro descritor do socket
 *
 */
void remover(int sockfd) {
	printf("\nRemover um filme\n");
	printf("\nInsira o id:\n");
	char* id = lerConsole();

	enviar(sockfd, id, (strlen(id) + 1) * sizeof(char));
	char* msg = receber(sockfd);
	if (strcmp(msg, "0") == 0) printf("\nFilme %s removido com sucesso\n", id);
	else printf("\nFilme %s nao pode ser removido ou nao existe\n", id);
}

/*
 * Funcao: escolheOperacao
 * -----------------------
 * Atribui um numero para cada operacao disponivel. Obtem do console a operacao escolhida pelo cliente, envia o 
 * codigo da operacao ao servidor e inicia o processamento chamando a funcao responsavel no lado cliente
 *
 * sockfd: inteiro descritor do socket
 *
 */
void escolheOperacao(int sockfd) {
	for ( ; ; ) {
		char* op = NULL;
		printf("\nServidor oferece as seguintes operacoes:\n");
		printf("1 - Cadastrar novo filme\n");
		printf("2 - Remover um filme\n");
		printf("3 - Listar titulo e salas de exibicao de todos os filmes\n");
		printf("4 - Listar todos os titulos de filmes de um determinado genero\n");
		printf("5 - Retornar o titulo de um filme\n");
		printf("6 - Retornar todas as informacoes de um filme\n");
		printf("7 - Listar todas as informacoes de todos os filmes\n");
		printf("8 - Sair\n");
		printf("\nEscolha a operacao pelo numero correspondente:\n");
		op = lerConsole();
		//printf("Operacao: %s\n", op);

		enviar(sockfd, op, 2);
		if (strcmp(op, "1") == 0) cadastrar(sockfd);
		else if (strcmp(op, "2") == 0) remover(sockfd); // TODO: Remover filme
		else if (strcmp(op, "3") == 0) {} // TODO: Listar titulo e salas de exibicao de todos os filmes
		else if (strcmp(op, "4") == 0) {} // TODO: Listar todos os titulos de filmes de um determinado genero
		else if (strcmp(op, "5") == 0) {} // TODO: Retornar o titulo de um filme
		else if (strcmp(op, "6") == 0) {} // TODO: Retornar todas as informacoes de um filme
		else if (strcmp(op, "7") == 0) {} // TODO: Listar todas as informacoes de todos os filmes
		else if (strcmp(op, "8") == 0) {
			printf("Encerrando\n");
			free(op);
			return;
		}
		else printf("Operacao indefinida\n");
		free(op);
	}
}

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

	printf("Conexao estabelecida\n");

	// TODO: operações no catálogo de filmes

	//func(sockfd);
	//cadastrar(sockfd);
	escolheOperacao(sockfd);

	exit(0);
}
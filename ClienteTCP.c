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

/* Programa cliente sobre conexão TCP */

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
	//printf("\nEnviando: %s (%d bytes)\n", buff, size);
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
	//printf("\nRecebendo: %s (%d bytes)\n", buff, size);
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
	titulo = receber(sockfd);
	sinopse = receber(sockfd);
	genero = receber(sockfd);
	salas = receber(sockfd);

	printf("\nNovo filme cadastrado com sucesso:\n");
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
 * Funcao: getTitulo
 * -----------------
 * Envia para o servidor o id de um filme recebe seu titulo
 *
 * sockfd: inteiro descritor do socket
 *
 */
void getTitulo(int sockfd) {
	printf("\nConsultar o titulo de um filme\n");
	printf("\nInsira o id\n");
	char* id = lerConsole();

	enviar(sockfd, id, (strlen(id) + 1) * sizeof(char));
	char* titulo = receber(sockfd);
	if (strcmp(titulo, "-1") == 0) printf("\nO filme %s nao existe\n", id);
	else printf("\nFilme % s possui titulo %s\n", id, titulo);
}

/*
 * Funcao: getTituloSalas
 * ----------------------
 * Recebe do servidor o titulo e salas de exibicao de todos os filmes
 *
 * sockfd: inteiro descritor do socket
 *
 */
void getTituloSalas(int sockfd) {
	printf("\nConsultando titulo e salas de exibicao de todos os filmes\n");

	char* linhas = receber(sockfd);
	//printf("\nO catalogo tem %s filmes\n", numFilmes);
	int numFilmes = atoi(linhas);
	printf("\nO catalogo tem %d filmes\n", numFilmes);

	char* titulo;
	char* salas;

	for (int i = 0; i < numFilmes; i++) {
		titulo = receber(sockfd);
		salas = receber(sockfd);
		printf("\n%d - Titulo: %s\tSalas: %s\n", i + 1, titulo, salas);
	}

	free(linhas);
	free(titulo);
	free(salas);
}

/*
 * Funcao: getTituloGenero
 * -----------------------
 * Cliente envia um genero e servidor retorna todos os titulos deste certo genero
 *
 * sockfd: inteiro descritor do socket
 *
 */
void getTituloGenero(int sockfd) {
	printf("\nListar todos os titulos de determinado genero\n");
	printf("\nInsira o genero\n");
	char* generoAlvo = lerConsole();

	enviar(sockfd, generoAlvo, (strlen(generoAlvo) + 1) * sizeof(char));

	char* linhas = receber(sockfd);
	//printf("\nO catalogo tem %s filmes\n", numFilmes);
	int numFilmes = atoi(linhas);
	printf("\nO catalogo tem %d filmes\n", numFilmes);

	char* titulo;
	char* genero;
	for (int i = 0; i < numFilmes; i++) {
		titulo = receber(sockfd);
		genero = receber(sockfd);
		if (strcmp(genero, generoAlvo) == 0) {
			printf("\n%d - Titulo: %s\tGenero: %s\n", i + 1, titulo, genero);
		}		
	}

	free(generoAlvo);
	free(linhas);
	free(titulo);
	free(genero);
}

/*
 * Funcao: getAll
 * --------------
 * Envia para o servidor o id de um filme recebe todas as suas informacoes
 *
 * sockfd: inteiro descritor do socket
 *
 */
void getAll(int sockfd) {
	printf("\nConsultar todas as informacoes de um filme\n");
	printf("\nInsira o id\n");
	char* id = lerConsole();

	enviar(sockfd, id, (strlen(id) + 1) * sizeof(char));
	char* idServ = receber(sockfd);
	char* titulo = receber(sockfd);
	char* sinopse = receber(sockfd);
	char* genero = receber(sockfd);
	char* salas = receber(sockfd);

	if (strcmp(idServ, "-1") == 0) printf("\nO filme %s nao existe\n", id);
	else {
		printf("\nFilme possui:\n");
		printf("Id: %s\n", id);
		printf("Titulo: %s\n", titulo);
		printf("Sinopse: %s\n", sinopse);
		printf("Genero: %s\n", genero);
		printf("Salas: %s\n", salas);
	}

	free(id);
	free(idServ);
	free(titulo);
	free(sinopse);
	free(genero);
	free(salas);
}

/*
 * Funcao: getCatalogo
 * -------------------
 * Servidor envia todas as informacoes de todos os filmes
 *
 * sockfd: inteiro descritor do socket
 *
 */
void getCatalogo(int sockfd) {
	printf("\nObter todas as informacoes de todos os filmes\n");

	char* linhas = receber(sockfd);
	//printf("\nO catalogo tem %s filmes\n", numFilmes);
	int numFilmes = atoi(linhas);
	printf("\nO catalogo tem %d filmes\n", numFilmes);

	char* id;
	char* titulo;
	char* sinopse;
	char* genero;
	char* salas;

	for (int i = 0; i < numFilmes; i++) {
		id = receber(sockfd);
		titulo = receber(sockfd);
		sinopse = receber(sockfd);
		genero = receber(sockfd);
		salas = receber(sockfd);
		printf("\n%d - Id: %s\tTitulo: %s\tSinopse: %s\tGenero: %s\tSalas: %s\n", i + 1, id, titulo, sinopse, genero, salas);
	}

	free(linhas);
	free(id);
	free(titulo);
	free(sinopse);
	free(genero);
	free(salas);
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
		else if (strcmp(op, "2") == 0) remover(sockfd);
		else if (strcmp(op, "3") == 0) getTituloSalas(sockfd);
		else if (strcmp(op, "4") == 0) getTituloGenero(sockfd);
		else if (strcmp(op, "5") == 0) getTitulo(sockfd);
		else if (strcmp(op, "6") == 0) getAll(sockfd);
		else if (strcmp(op, "7") == 0) getCatalogo(sockfd); // TODO: Listar todas as informacoes de todos os filmes
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
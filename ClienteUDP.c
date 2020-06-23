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

/* Programa cliente sobre protocolo UDP */

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
void enviar(int sockfd, const char* buff, size_t size, const struct sockaddr* addr, socklen_t addrlen) {
	sendto(sockfd, &size, sizeof(size_t), 0, addr, addrlen);
	sendto(sockfd, buff, size, 0, addr, addrlen);
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
char* receber(int sockfd, struct sockaddr* addr, int* addrlen) {
	size_t size;
	recvfrom(sockfd, &size, sizeof(size_t), 0, addr, addrlen);
	char* buff = (void*)malloc(size * sizeof(char));
	recvfrom(sockfd, buff, size, 0, addr, addrlen);
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
void cadastrar(int sockfd, struct sockaddr* addr, int addrlen) {
	printf("\nCadastrar novo filme:\n");
	printf("\nInsira o titulo:\n");
	char* titulo = lerConsole();

	printf("\nInsira a sinopse:\n");
	char* sinopse = lerConsole();

	printf("\nInsira o genero:\n");
	char* genero = lerConsole();

	printf("\nInsira as salas:\n");
	char* salas = lerConsole();

	//int addrlen = sizeof(addr);

	enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char), addr, addrlen);
	enviar(sockfd, sinopse, (strlen(sinopse) + 1) * sizeof(char), addr, addrlen);
	enviar(sockfd, genero, (strlen(genero) + 1) * sizeof(char), addr, addrlen);
	enviar(sockfd, salas, (strlen(salas) + 1) * sizeof(char), addr, addrlen);

	char* id = receber(sockfd, addr, &addrlen);
	titulo = receber(sockfd, addr, &addrlen);
	sinopse = receber(sockfd, addr, &addrlen);
	genero = receber(sockfd, addr, &addrlen);
	salas = receber(sockfd, addr, &addrlen);

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
void remover(int sockfd, struct sockaddr* addr, int addrlen) {
	printf("\nRemover um filme\n");
	printf("\nInsira o id:\n");
	char* id = lerConsole();

	enviar(sockfd, id, (strlen(id) + 1) * sizeof(char), addr, addrlen);
	char* msg = receber(sockfd, addr, &addrlen);
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
void getTitulo(int sockfd, struct sockaddr* addr, int addrlen) {
	printf("\nConsultar o titulo de um filme\n");
	printf("\nInsira o id\n");
	char* id = lerConsole();

	enviar(sockfd, id, (strlen(id) + 1) * sizeof(char), addr, addrlen);
	char* titulo = receber(sockfd, addr, &addrlen);
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
void getTituloSalas(int sockfd, struct sockaddr* addr, int addrlen) {
	printf("\nConsultando titulo e salas de exibicao de todos os filmes\n");

	char* linhas = receber(sockfd, addr, &addrlen);
	//printf("\nO catalogo tem %s filmes\n", numFilmes);
	int numFilmes = atoi(linhas);
	printf("\nO catalogo tem %d filmes\n", numFilmes);

	char* titulo;
	char* salas;

	for (int i = 0; i < numFilmes; i++) {
		titulo = receber(sockfd, addr, &addrlen);
		salas = receber(sockfd, addr, &addrlen);
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
void getTituloGenero(int sockfd, struct sockaddr* addr, int addrlen) {
	printf("\nListar todos os titulos de determinado genero\n");
	printf("\nInsira o genero\n");
	char* generoAlvo = lerConsole();

	enviar(sockfd, generoAlvo, (strlen(generoAlvo) + 1) * sizeof(char), addr, addrlen);

	char* linhas = receber(sockfd, addr, &addrlen);
	//printf("\nO catalogo tem %s filmes\n", numFilmes);
	int numFilmes = atoi(linhas);
	printf("\nO catalogo tem %d filmes\n", numFilmes);

	char* titulo;
	char* genero;
	int existe = 0;
	for (int i = 0; i < numFilmes; i++) {
		titulo = receber(sockfd, addr, &addrlen);
		genero = receber(sockfd, addr, &addrlen);
		if (strcmp(genero, generoAlvo) == 0) {
			printf("\n%d - Titulo: %s\tGenero: %s\n", i + 1, titulo, genero);
			existe = 1;
		}
		//else printf("\nNao existem filmes do genero %s cadastrados\n", generoAlvo);
	}

	if(existe == 0) printf("\nNao existem filmes do genero %s cadastrados\n", generoAlvo);

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
void getAll(int sockfd, struct sockaddr* addr, int addrlen) {
	printf("\nConsultar todas as informacoes de um filme\n");
	printf("\nInsira o id\n");
	char* id = lerConsole();

	enviar(sockfd, id, (strlen(id) + 1) * sizeof(char), addr, addrlen);
	char* idServ = receber(sockfd, addr, &addrlen);
	char* titulo = receber(sockfd, addr, &addrlen);
	char* sinopse = receber(sockfd, addr, &addrlen);
	char* genero = receber(sockfd, addr, &addrlen);
	char* salas = receber(sockfd, addr, &addrlen);

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
void getCatalogo(int sockfd, struct sockaddr* addr, int addrlen) {
	printf("\nObter todas as informacoes de todos os filmes\n");

	char* linhas = receber(sockfd, addr, &addrlen);
	//printf("\nO catalogo tem %s filmes\n", numFilmes);
	int numFilmes = atoi(linhas);
	printf("\nO catalogo tem %d filmes\n", numFilmes);

	char* id;
	char* titulo;
	char* sinopse;
	char* genero;
	char* salas;

	for (int i = 0; i < numFilmes; i++) {
		id = receber(sockfd, addr, &addrlen);
		titulo = receber(sockfd, addr, &addrlen);
		sinopse = receber(sockfd, addr, &addrlen);
		genero = receber(sockfd, addr, &addrlen);
		salas = receber(sockfd, addr, &addrlen);
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
 * codigo da operacao ao servidor e inicia o processamento chamando a funcao responsavel no lado cliente. Em seguida recebe do servidor
 * a variavel status que informa se existem filmes cadastrados no catálogo de filmes ou não, a fim de tratar o caso de lista vazia onde somente
 * a operacao de cadastro podera ser realizada.
 *
 * sockfd: inteiro descritor do socket
 *
 */
void escolheOperacao(int sockfd, struct sockaddr* addr, int addrlen) {
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

		enviar(sockfd, op, 2, addr, addrlen);
		if (strcmp(op, "1") == 0) cadastrar(sockfd, addr, addrlen);
		else if (strcmp(op, "8") == 0) {
			printf("Encerrando\n");
			free(op);
			return;
		}
		else {
			char* status = receber(sockfd, addr, &addrlen);
			if (strcmp(status, "0") == 0) {
				printf("\nO catalogo de filmes esta vazio, somente a operacao de cadastro pode ser realizada\n");
			}
			else {
				if (strcmp(op, "2") == 0) remover(sockfd, addr, addrlen);
				else if (strcmp(op, "3") == 0) getTituloSalas(sockfd, addr, addrlen);
				else if (strcmp(op, "4") == 0) getTituloGenero(sockfd, addr, addrlen);
				else if (strcmp(op, "5") == 0) getTitulo(sockfd, addr, addrlen);
				else if (strcmp(op, "6") == 0) getAll(sockfd, addr, addrlen);
				else if (strcmp(op, "7") == 0) getCatalogo(sockfd, addr, addrlen);
				else printf("Operacao indefinida\n");
			}
			free(status);
		}
		free(op);
	}
}

int main(int argc, char** argv) {
	int sockfd;
	struct sockaddr_in addr;

	if (argc != 2) {
		perror("usage: tcpcli <IPaddress>");
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket error");
		exit(1);
	}

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERV_PORT);
	if (inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0) {
		perror("inet_pton error");
		exit(1);
	}

	int addrlen = sizeof(addr);

	// Operações no catálogo de filmes
	escolheOperacao(sockfd, (struct sockaddr*)&addr, addrlen);

	close(sockfd);

	exit(0);
}
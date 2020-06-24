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
#include <poll.h>

#define SERV_PORT 3490
#define TIMEOUT 60000

static void escolheOperacao(int sockfd, struct sockaddr* addr, int addrlen);

/* Programa cliente sobre protocolo UDP */

/*
 * Funcao: enviar
 * --------------
 * Encapsula 2 chamadas de sendTo para enviar o tamanho da variavel
 * antes do envio dos dados
 *
 * sockfd: inteiro descritor do socket
 * buff: ponteiro para variavel que armazena os dados
 * size: numero de bytes a enviar
 * addr: ponteiro para a estrutura que contem o endereço IP e porta
 * addrlen: inteiro referente ao tamanho da estrutura acima
 *
 */
void enviar(int sockfd, const char* buff, size_t size, const struct sockaddr* addr, socklen_t addrlen) {
	sendto(sockfd, &size, sizeof(size_t), 0, addr, addrlen);
	sendto(sockfd, buff, size, 0, addr, addrlen);
	//printf("\nEnviando: %s (%d bytes)\n", buff, size);
}

/*
 * Funcao: checkSocket
 * -------------------
 * Utiliza a primitiva poll para verificar se um socket tem
 * dados a ler em um intervalo TIMEOUT definido como constante no topo do programa. Caso tenha, a funcao
 * retorna e o programa continua normalmente.
 * Caso contrario, escolheOperacao é chamada e o cliente aguarda que o usuario solicite uma
 * nova operacao.
 *
 * sockfd: inteiro descritor do socket
 * addr: ponteiro para a estrutura que contem o endereço IP e porta
 * addrlen: inteiro referente ao tamanho da estrutura acima
 *
 */
void checkSocket(int sockfd, struct sockaddr* addr, int addrlen) {
	struct pollfd pfds[1];
	pfds[0].fd = sockfd;
	pfds[0].events = POLLIN;
	int num_events = poll(pfds, 1, TIMEOUT);

	if (num_events == 0) {
		printf("\nTempo limite excedido\n");
		escolheOperacao(sockfd, (struct sockaddr*)&addr, addrlen);
	}
	else {
		int pollin_happened = pfds[0].revents & POLLIN;
		if (pollin_happened) {
			//printf("\nSocket %d esta pronto para ler\n", pfds[0].fd);
			return;
		}
		else {
			printf("\nEvento inesperado: %d\n", pfds[0].revents);
			escolheOperacao(sockfd, (struct sockaddr*)&addr, addrlen);
		}
	}
}

/*
 * Funcao: receber
 * ---------------
 * Encapsula 2 chamadas de checkSocket para verificar se existem dados a ler, caso nao existam
 * dados no socket, checkSocket chama novamente escolheOperacao para aguarda uma nova mensagem.
 * Se houver dados a ler, a chamada recvfrom prossegue. A primeira chamada de recvfrom recebe o tamanho
 * do dado para realizar a alocacao dinamica da variavel e segunda chamada recebe o dado em si.
 *
 * sockfd: inteiro descritor do socket
 * addr: ponteiro para a estrutura que contem o endereço IP e porta
 * addrlen: inteiro referente ao tamanho da estrutura acima
 *
 * retorna: ponteiro buff para os dados recebidos
 */
char* receber(int sockfd, struct sockaddr* addr, int* addrlen) {
	size_t size;
	checkSocket(sockfd, (struct sockaddr*)&addr, *addrlen);
	recvfrom(sockfd, &size, sizeof(size_t), 0, addr, addrlen);
	char* buff = (void*)malloc(size * sizeof(char));
	checkSocket(sockfd, (struct sockaddr*)&addr, *addrlen);
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
 * Cadastra um novo filme a partir do titulo, sinopse, genero e
 * salas recebidos do cliente, envia de volta um identificador unico
 *
 * sockfd: inteiro descritor do socket
 * addr: ponteiro para a estrutura que contem o endereço IP e porta
 * addrlen: inteiro referente ao tamanho da estrutura acima
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
 * addr: ponteiro para a estrutura que contem o endereço IP e porta
 * addrlen: inteiro referente ao tamanho da estrutura acima
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
 * Recebe do cliente um id e envia o titulo do filme correspondente
 *
 * sockfd: inteiro descritor do socket
 * addr: ponteiro para a estrutura que contem o endereço IP e porta
 * addrlen: inteiro referente ao tamanho da estrutura acima
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
 * Envia ao cliente o titulo e as salas de exibicao de todos os filmes
 *
 * sockfd: inteiro descritor do socket
 * addr: ponteiro para a estrutura que contem o endereço IP e porta
 * addrlen: inteiro referente ao tamanho da estrutura acima
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
 * Servidor recebe um genero e retorna todos os titulos deste certo genero
 *
 * sockfd: inteiro descritor do socket
 * addr: ponteiro para a estrutura que contem o endereço IP e porta
 * addrlen: inteiro referente ao tamanho da estrutura acima
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
 * Recebe do cliente um id e enviar todas as informacoes do filme
 *
 * sockfd: inteiro descritor do socket
 * addr: ponteiro para a estrutura que contem o endereço IP e porta
 * addrlen: inteiro referente ao tamanho da estrutura acima
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
 * addr: ponteiro para a estrutura que contem o endereço IP e porta
 * addrlen: inteiro referente ao tamanho da estrutura acima
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
 * Cliente envia ao servidor a operacao que deseja realizar. Servidor chama funcao que conta o numero
 * de linhas do catalogo de filmes, para tratar o caso de ainda não existirem filmes cadastrados. Se a lista de filmes
 * ainda não existir ou estiver vazia, a variavel status = 0, caso contrário status = 1. Se ainda não houver filmes
 * cadastrados, somente as operacoes 1 e 8 podem ser enviadas ao servidor.
 *
 * sockfd: inteiro descritor do socket
 * addr: ponteiro para a estrutura que contem o endereço IP e porta
 * addrlen: inteiro referente ao tamanho da estrutura acima
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

/* Neste programa, main e responsavel por chamar as primitivas de sockets referentes
   ao protocolo UDP e em seguida chamar a funcao escolheOperacao que define qual operacao
   sera executada de acordo com o cliente */
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
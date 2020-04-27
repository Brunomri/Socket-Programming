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
#define SERV_PORT 3490

// Estrutura filme com título, sinopse, gênero, salas em exibição e identificador único
//typedef struct filmes
//{
//    char* id;
//    char* titulo;
//    char* sinopse;
//    char* genero;
//   char* salas;
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
 * Funcao: criarID
 * ---------------
 * Cria um identificador unico para atribuir a um novo filme
 *
 * retorna: ponteiro para identificador unico
 */
char* criarID() {
    srand(time(0));
    int idTemp = rand();
    char* id = (char*) malloc (sizeof(idTemp));
    sprintf(id, "%d", idTemp);
    printf("\nnovo id: %s (%d bytes)\n", id, (strlen(id) + 1) * sizeof(char));
    return id;
}

/*
 * Funcao: cadastrar
 * -----------------
 * Cadastra um novo filme a partir do titulo, sinopse, genero e
 * salas recebidos do cliente, envia de volta um identificador unico
 *
 * sockfd: inteiro descritor do socket
 *
 */
void cadastrar(int sockfd) {
    printf("\nCadastrar novo filme:\n");

    char *titulo = receber(sockfd);
    char *sinopse = receber(sockfd);
    char *genero = receber(sockfd);
    char *salas = receber(sockfd);

    char* id = criarID();
    //printf("\nnovo id: %s\n", id);

    FILE* fp;
    if ((fp = fopen(id, "w")) != NULL) {
        int ret;
        fputs(id, fp);
        fputs("\n", fp);
        fputs(titulo, fp);
        fputs("\n", fp);
        fputs(sinopse, fp);
        fputs("\n", fp);
        fputs(genero, fp);
        fputs("\n", fp);
        fputs(salas, fp);
        
        if ((ret = fclose(fp)) == 0) {
            printf("\nNovo filme cadastrado com sucesso:\n");
            printf("Id: %s\n", id);
            printf("Titulo: %s\n", titulo);
            printf("Sinopse: %s\n", sinopse);
            printf("Genero: %s\n", genero);
            printf("Salas: %s\n", salas);

            enviar(sockfd, id, (strlen(id) + 1) * sizeof(char));
            enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char));
            enviar(sockfd, sinopse, (strlen(sinopse) + 1) * sizeof(char));
            enviar(sockfd, genero, (strlen(genero) + 1) * sizeof(char));
            enviar(sockfd, salas, (strlen(salas) + 1) * sizeof(char));
            /* O tamanho em bytes do id e o seu comprimento
            retornado por strlen() acrescido de 1, para considerar
            o caractere de final de cadeia, multiplicado pelo
            tamanho em bytes de um char */
        }
        else {
            char* msg = "\nO arquivo do novo filme nao pode ser criado\n";
            enviar(sockfd, msg, (strlen(msg) + 1) * sizeof(char));
        }

    }
    else {
        char* msg = "\nO arquivo do novo filme nao pode ser criado\n";
        enviar(sockfd, msg, (strlen(msg) + 1) * sizeof(char));
    }

    free(id);
    free(titulo);
    free(sinopse);
    free(genero);
    free(salas);
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
    char* id = receber(sockfd);
    printf("\nRemovendo filme %s\n", id);

    int ret = remove(id);
    char* res;
    if (ret == 0) {
        printf("\nFilme %s removido com sucesso\n", id);
        res = "0";
    }
    else {
        printf("\nFilme %s nao pode ser removido\n", id);
        res = "-1";
    }
    enviar(sockfd, res, (strlen(res) + 1) * sizeof(char));
}

/*
 * Funcao: escolheOperacao
 * -----------------------
 * Recebe do cliente a operacao escolhida e inicia o processamento
 *
 * sockfd: inteiro descritor do socket
 *
 */
void escolheOperacao(int sockfd) {
    for (; ; ) {
        printf("\nRecebendo operacao do cliente\n");
        char* op = receber(sockfd);
        printf("\nExecutando operacao %s\n", op);

        if (strcmp(op, "1") == 0) cadastrar(sockfd);
        else if (strcmp(op, "2") == 0) remover(sockfd); // TODO: Remover filme
        else if (strcmp(op, "3") == 0) {} // TODO: Listar titulo e salas de exibicao de todos os filmes
        else if (strcmp(op, "4") == 0) {} // TODO: Listar todos os titulos de filmes de um determinado genero
        else if (strcmp(op, "5") == 0) {} // TODO: Retornar o titulo de um filme
        else if (strcmp(op, "6") == 0) {} // TODO: Retornar todas as informacoes de um filme
        else if (strcmp(op, "7") == 0) {} // TODO: Listar todas as informacoes de todos os filmes
        else if (strcmp(op, "8") == 0) {
            printf("Cliente encerrou conexao\n");
            exit(0);
        }
    }
}

int main(int argc, char** argv) {
    int    listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in servaddr, cliaddr;
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

    printf("Servidor aguardando conexoes\n");

    for ( ; ; ) {
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen)) == -1) {
            perror("accept");
            exit(1);
        }

        if ((childpid = fork()) == 0) {
            close(listenfd);
            // TODO: operações no catálogo de filmes
            //cadastrar(connfd);
            escolheOperacao(connfd);
            close(connfd);
            exit(0);
        }

        close(connfd);
        //break;
    }
    return(0);
}
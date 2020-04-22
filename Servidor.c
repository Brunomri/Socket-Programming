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
#define MAXDATASIZE 500
#define SERV_PORT 3490

void func(int sockfd)
{
    char buff[MAXDATASIZE];
    int n;
    // infinite loop for chat 
    for (;;) {
        bzero(buff, MAXDATASIZE);

        // read the message from client and copy it in buffer 
        read(sockfd, buff, sizeof(buff));
        // print buffer which contains the client contents 
        printf("From client: %s\t To client : ", buff);
        bzero(buff, MAXDATASIZE);
        n = 0;
        // copy server message in the buffer 
        while ((buff[n++] = getchar()) != '\n')
            ;

        // and send that buffer to client 
        write(sockfd, buff, sizeof(buff));

        // if msg contains "Exit" then server exit and chat ended. 
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }
}

// Estrutura filme com título, sinopse, gênero, salas em exibição e identificador único
//typedef struct filmes
//{
//    char* id;
//    char* titulo;
//    char* sinopse;
//    char* genero;
//   char* salas;
//} filme;

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

void enviar(int sockfd, const void* buff) {
    writen(sockfd, buff, MAXDATASIZE);
    printf("Enviando: %s\n", buff);
}

void receber(int sockfd, void* buff) {
    readn(sockfd, buff, MAXDATASIZE);
    printf("Recebendo: %s\n", buff);
}

// Cria um ID para ser atribuído a um novo filme
int criarID() {
    srand(time(0));
    int id = rand();
    //printf("novo id: %s", id);
    return id;
}

void cadastrar(int sockfd) {
    char titulo[MAXDATASIZE];
    receber(sockfd, titulo);

    char sinopse[MAXDATASIZE];
    receber(sockfd, sinopse);

    char genero[MAXDATASIZE];
    receber(sockfd, genero);

    char salas[MAXDATASIZE];
    receber(sockfd, salas);

    int idTemp = criarID();
    char id[MAXDATASIZE];
    sprintf(id, "%d", idTemp);

    FILE* fp;
    fp = fopen(id, "w");
    fputs(id, fp);
    fputs("\n", fp);
    fputs(titulo, fp);
    fputs("\n", fp);
    fputs(sinopse, fp);
    fputs("\n", fp);
    fputs(genero, fp);
    fputs("\n", fp);
    fputs(salas, fp);
    fclose(fp);

    printf("\nnovo filme cadastrado:\n");
    printf("id: %s\n", id);
    printf("titulo: %s\n", titulo);
    printf("sinopse: %s\n", sinopse);
    printf("genero: %s\n", genero);
    printf("salas: %s\n", salas);

    enviar(sockfd, id);
}

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

    printf("Servidor aguardando conexoes\n");

    for (; ; ) {
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen)) == -1) {
            perror("accept");
            exit(1);
        }

        if ((childpid = fork()) == 0) {
            close(listenfd);
            // TODO: operações no catálogo de filmes
            cadastrar(connfd);
            exit(0);
        }

        close(connfd);
    }
    return(0);
}
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
 * Funcao: lerArquivo
 * ------------------
 * Faz a leitura de uma linha em arquivo um caractere por vez para
 * alocar dinamicamente a memoria necessaria para as variaveis
 *
 * retorna: sequencia de caracteres da linha
 */
char* lerArquivo(FILE* fp) {
    char* line = NULL, * tmp = NULL;
    size_t size = 0, index = 0;
    int ch = EOF;

    while (ch) {
        ch = fgetc(fp);

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
 * Funcao: isEmpty
 * ---------------
 * Determina se uma sequencia de caracteres e vazia
 *
 * str: Sequencia de caracteres para analisar
 *
 * retorna: 1 se vazia, caso contrario 0
 */
int isEmpty(const char* str)
{
    char ch;

    do
    {
        ch = *(str++);

        // Verifica caracteres em branco
        if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r' && ch != '\0')
            return 0;

    } while (ch != '\0');

    return 1;
}

/*
 * Funcao: contaLinhas
 * -------------------
 * Determina o numero de linhas em um arquivo
 *
 * file: Arquivo para analisar
 *
 * retorna: numero de linhas do arquivo
 */
int contaLinhas(char* file) {
    FILE* fp;
    char ch;
    char ant;
    int numLinhas = 0;

    if ((fp = fopen(file, "r")) == NULL) {
        printf("\nO arquivo %s nao pode ser aberto\n", file);
        return -1;
    }

    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            if (ant == ch) {
                break;
            }
            numLinhas++;
        }
        ant = ch;
    }
    fclose(fp);
    printf("\nO numero de linhas e %d\n", numLinhas);
    
    return numLinhas;
}

/*
 * Funcao: lerFilme
 * ----------------
 * Recebe o id de um filme e qual parametro dentre id, titulo, sinopse, genero ou
 * salas e retorna o parametro escolhido do filme correspondente
 *
 */
char* lerFilme(char* id, int param) {
    FILE* fp;
    if ((fp = fopen(id, "r")) != NULL) {
        id = lerArquivo(fp);
        char* titulo = lerArquivo(fp);
        char* sinopse = lerArquivo(fp);
        char* genero = lerArquivo(fp);
        char* salas = lerArquivo(fp);
        fclose(fp);
        
        printf("\nFilme %s possui ", id);
        if (param == 0) {
            printf("id: %s\n", id);
            return id;
        }
        else if (param == 1) {
            printf("titulo: %s\n", titulo);
            return titulo;
        }
        else if (param == 2) {
            printf("sinopse: %s\n", sinopse);
            return sinopse;
        }
        else if (param == 3) {
            printf("genero: %s\n", genero);
            return genero;
        }
        else if (param == 4) {
            printf("salas: %s\n", salas);
            return salas;
        }
        else printf("\nErro: Parametros devem ser inteiros de 0 a 4\n");
    }
    else {
        printf("\nO filme %s nao existe\n", id);
        return "-1";
    }
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
    int ret;
    if ((fp = fopen(id, "w")) != NULL) {
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

    if ((fp = fopen("listaFilmes", "a")) != NULL) {
        fputs(id, fp);
        fputs("\n", fp);
        if ((ret = fclose(fp)) == 0) printf("\nFilme %s adicionado a lista\n", id);
        else printf("\nFilme %s nao pode ser adicionado a lista\n");
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

    FILE *fp1, *fp2;
    if ((fp1 = fopen("listaFilmes", "r")) == NULL) {
        printf("\nA lista de filmes nao pode ser aberta\n");
    }
    if ((fp2 = fopen("temp", "w")) == NULL) {
        printf("\nA lista temporaria nao pode ser aberta\n");
    }
    char* linha;
    while (!feof(fp1)) {
        linha = lerArquivo(fp1);
        if ((strcmp(linha, id) != 0) && !isEmpty(linha)) {
            fputs(linha, fp2);
            fputs("\n", fp2);
        }
    }
    fclose(fp1);
    fclose(fp2);
    remove("listaFilmes");
    rename("temp", "listaFilmes");
}

/*
 * Funcao: getTitulo
 * -----------------
 * Recebe do cliente um id e envia o titulo do filme correspondente
 *
 * sockfd: inteiro descritor do socket
 *
 */
void getTitulo(int sockfd) {
    char* id = receber(sockfd);
    printf("\nConsultando filme %s\n", id);
    char* titulo = lerFilme(id, 1);

    enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char));
}

/*
 * Funcao: getTituloSalas
 * ----------------------
 * Envia ao cliente o titulo e as salas de exibicao de todos os filmes
 *
 * sockfd: inteiro descritor do socket
 *
 */
void getTituloSalas(int sockfd) {
    printf("\nConsultando titulo e salas de exibicao de todos os filmes\n");

    int numFilmes = contaLinhas("listaFilmes");
    //printf("\nO catalogo tem %d filmes\n", numFilmes);

    char* linhas = (char*)malloc(sizeof(numFilmes));
    sprintf(linhas, "%d", numFilmes);
    printf("\nO catalogo tem %s filmes\n", linhas);

    enviar(sockfd, linhas, (strlen(linhas) + 1) * sizeof(char));

    FILE* fp;
    if ((fp = fopen("listaFilmes", "r")) == NULL) printf("\nA lista de filmes nao pode ser aberta\n");
    else {
        char* id;
        char* titulo;
        char* salas;
        for (int i = 0; i < numFilmes; i++) {
            id = lerArquivo(fp);
            titulo = lerFilme(id, 1);
            salas = lerFilme(id, 4);
            printf("\nFilme %s tem titulo %s e salas %s\n", id, titulo, salas);

            enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char));
            enviar(sockfd, salas, (strlen(salas) + 1) * sizeof(char));
        }
        free(id);
        free(titulo);
        free(salas);
    }
    free(linhas);
}

void getTituloGenero(int sockfd) {
    char* generoAlvo = receber(sockfd);
    printf("\nObter todos os titulos com genero %s\n", generoAlvo);

    int numFilmes = contaLinhas("listaFilmes");
    char* linhas = (char*)malloc(sizeof(numFilmes));
    sprintf(linhas, "%d", numFilmes);
    printf("\nO catalogo tem %s filmes\n", linhas);

    enviar(sockfd, linhas, (strlen(linhas) + 1) * sizeof(char));

    FILE* fp;
    if ((fp = fopen("listaFilmes", "r")) == NULL) printf("\nA lista de filmes nao pode ser aberta\n");
    else {
        char* id;
        char* titulo;
        char* genero;
        for (int i = 0; i < numFilmes; i++) {
            id = lerArquivo(fp);
            titulo = lerFilme(id, 1);
            genero = lerFilme(id, 3);
            //if (strcmp(genero, generoAlvo) == 0) {
            //    printf("\nFilme %s tem titulo %s e genero %s\n", id, titulo, genero);
            //    enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char));
            //    enviar(sockfd, genero, (strlen(genero) + 1) * sizeof(char));
            //}
            //else printf("\nTitulo %s nao faz parte do genero %s\n", titulo, genero);
            printf("\nFilme %s tem titulo %s e genero %s\n", id, titulo, genero);
            enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char));
            enviar(sockfd, genero, (strlen(genero) + 1) * sizeof(char));
        }
        free(id);
        free(titulo);
        free(genero);
    }
    free(generoAlvo);
    free(linhas);
}

/*
 * Funcao: getAll
 * --------------
 * Recebe do cliente um id e enviar todas as informacoes do filme
 *
 * sockfd: inteiro descritor do socket
 *
 */
void getAll(int sockfd) {
    char* id = receber(sockfd);
    printf("\nConsultando filme %s\n", id);
    
    id = lerFilme(id, 0);
    char* titulo = lerFilme(id, 1);
    char* sinopse = lerFilme(id, 2);
    char* genero = lerFilme(id, 3);
    char* salas = lerFilme(id, 4);

    enviar(sockfd, id, (strlen(id) + 1) * sizeof(char));
    enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char));
    enviar(sockfd, sinopse, (strlen(sinopse) + 1) * sizeof(char));
    enviar(sockfd, genero, (strlen(genero) + 1) * sizeof(char));
    enviar(sockfd, salas, (strlen(salas) + 1) * sizeof(char));
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
        else if (strcmp(op, "2") == 0) remover(sockfd);
        else if (strcmp(op, "3") == 0) getTituloSalas(sockfd);
        else if (strcmp(op, "4") == 0) getTituloGenero(sockfd);
        else if (strcmp(op, "5") == 0) getTitulo(sockfd);
        else if (strcmp(op, "6") == 0) getAll(sockfd);
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
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

/* Programa servidor iterativo sobre protocolo UDP */

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
 * Funcao: criarID
 * ---------------
 * Cria um identificador unico para atribuir a um novo filme
 *
 * retorna: ponteiro para identificador unico
 */
char* criarID() {
    srand(time(0));
    int idTemp = rand();
    char* id = (char*)malloc(sizeof(idTemp));
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
void cadastrar(int sockfd, struct sockaddr* addr, int addrlen) {
    printf("\nCadastrar novo filme:\n");

    char* titulo = receber(sockfd, addr, &addrlen);
    char* sinopse = receber(sockfd, addr, &addrlen);
    char* genero = receber(sockfd, addr, &addrlen);
    char* salas = receber(sockfd, addr, &addrlen);

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

            enviar(sockfd, id, (strlen(id) + 1) * sizeof(char), addr, addrlen);
            enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char), addr, addrlen);
            enviar(sockfd, sinopse, (strlen(sinopse) + 1) * sizeof(char), addr, addrlen);
            enviar(sockfd, genero, (strlen(genero) + 1) * sizeof(char), addr, addrlen);
            enviar(sockfd, salas, (strlen(salas) + 1) * sizeof(char), addr, addrlen);
            /* O tamanho em bytes do id e o seu comprimento
            retornado por strlen() acrescido de 1, para considerar
            o caractere de final de cadeia, multiplicado pelo
            tamanho em bytes de um char */
        }
        else {
            char* msg = "\nO arquivo do novo filme nao pode ser criado\n";
            enviar(sockfd, msg, (strlen(msg) + 1) * sizeof(char), addr, addrlen);
        }

    }
    else {
        char* msg = "\nO arquivo do novo filme nao pode ser criado\n";
        enviar(sockfd, msg, (strlen(msg) + 1) * sizeof(char), addr, addrlen);
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
    fclose(fp);
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
    char* id = receber(sockfd, addr, &addrlen);
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
    enviar(sockfd, res, (strlen(res) + 1) * sizeof(char), addr, addrlen);

    FILE* fp1, * fp2;
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
void getTitulo(int sockfd, struct sockaddr* addr, int addrlen) {
    char* id = receber(sockfd, addr, &addrlen);
    printf("\nConsultando filme %s\n", id);
    char* titulo = lerFilme(id, 1);

    enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char), addr, addrlen);
}

/*
 * Funcao: getTituloSalas
 * ----------------------
 * Envia ao cliente o titulo e as salas de exibicao de todos os filmes
 *
 * sockfd: inteiro descritor do socket
 *
 */
void getTituloSalas(int sockfd, struct sockaddr* addr, int addrlen) {
    printf("\nConsultando titulo e salas de exibicao de todos os filmes\n");

    int numFilmes = contaLinhas("listaFilmes");
    //printf("\nO catalogo tem %d filmes\n", numFilmes);

    char* linhas = (char*)malloc(sizeof(numFilmes));
    sprintf(linhas, "%d", numFilmes);
    printf("\nO catalogo tem %s filmes\n", linhas);

    enviar(sockfd, linhas, (strlen(linhas) + 1) * sizeof(char), addr, addrlen);

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

            enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char), addr, addrlen);
            enviar(sockfd, salas, (strlen(salas) + 1) * sizeof(char), addr, addrlen);
        }
        free(id);
        free(titulo);
        free(salas);
    }
    free(linhas);
    fclose(fp);
}

/*
 * Funcao: getTituloGenero
 * -----------------------
 * Servidor recebe um genero e retorna todos os titulos deste certo genero
 *
 * sockfd: inteiro descritor do socket
 *
 */
void getTituloGenero(int sockfd, struct sockaddr* addr, int addrlen) {
    char* generoAlvo = receber(sockfd, addr, &addrlen);
    printf("\nObter todos os titulos com genero %s\n", generoAlvo);

    int numFilmes = contaLinhas("listaFilmes");
    char* linhas = (char*)malloc(sizeof(numFilmes));
    sprintf(linhas, "%d", numFilmes);
    //printf("\nO catalogo tem %s filmes\n", linhas);

    enviar(sockfd, linhas, (strlen(linhas) + 1) * sizeof(char), addr, addrlen);

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
            //    enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char), addr, addrlen);
            //    enviar(sockfd, genero, (strlen(genero) + 1) * sizeof(char), addr, addrlen);
            //}
            //else printf("\nTitulo %s nao faz parte do genero %s\n", titulo, genero);
            printf("\nFilme %s tem titulo %s e genero %s\n", id, titulo, genero);
            enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char), addr, addrlen);
            enviar(sockfd, genero, (strlen(genero) + 1) * sizeof(char), addr, addrlen);
        }
        free(id);
        free(titulo);
        free(genero);
    }
    free(generoAlvo);
    free(linhas);
    fclose(fp);
}

/*
 * Funcao: getAll
 * --------------
 * Recebe do cliente um id e enviar todas as informacoes do filme
 *
 * sockfd: inteiro descritor do socket
 *
 */
void getAll(int sockfd, struct sockaddr* addr, int addrlen) {
    char* id = receber(sockfd, addr, &addrlen);
    printf("\nConsultando filme %s\n", id);

    id = lerFilme(id, 0);
    char* titulo = lerFilme(id, 1);
    char* sinopse = lerFilme(id, 2);
    char* genero = lerFilme(id, 3);
    char* salas = lerFilme(id, 4);

    enviar(sockfd, id, (strlen(id) + 1) * sizeof(char), addr, addrlen);
    enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char), addr, addrlen);
    enviar(sockfd, sinopse, (strlen(sinopse) + 1) * sizeof(char), addr, addrlen);
    enviar(sockfd, genero, (strlen(genero) + 1) * sizeof(char), addr, addrlen);
    enviar(sockfd, salas, (strlen(salas) + 1) * sizeof(char), addr, addrlen);
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
    printf("\nConsultando todas as informações de todos os filmes\n");

    int numFilmes = contaLinhas("listaFilmes");
    //printf("\nO catalogo tem %d filmes\n", numFilmes);

    char* linhas = (char*)malloc(sizeof(numFilmes));
    sprintf(linhas, "%d", numFilmes);
    printf("\nO catalogo tem %s filmes\n", linhas);

    enviar(sockfd, linhas, (strlen(linhas) + 1) * sizeof(char), addr, addrlen);

    FILE* fp;
    if ((fp = fopen("listaFilmes", "r")) == NULL) printf("\nA lista de filmes nao pode ser aberta\n");
    else {
        char* id;
        char* titulo;
        char* sinopse;
        char* genero;
        char* salas;
        for (int i = 0; i < numFilmes; i++) {
            id = lerArquivo(fp);
            titulo = lerFilme(id, 1);
            sinopse = lerFilme(id, 2);
            genero = lerFilme(id, 3);
            salas = lerFilme(id, 4);
            printf("\n%d - Id: %s\tTitulo: %s\tSinopse: %s\tGenero: %s\tSalas: %s\n", i + 1, id, titulo, sinopse, genero, salas);

            enviar(sockfd, id, (strlen(id) + 1) * sizeof(char), addr, addrlen);
            enviar(sockfd, titulo, (strlen(titulo) + 1) * sizeof(char), addr, addrlen);
            enviar(sockfd, sinopse, (strlen(sinopse) + 1) * sizeof(char), addr, addrlen);
            enviar(sockfd, genero, (strlen(genero) + 1) * sizeof(char), addr, addrlen);
            enviar(sockfd, salas, (strlen(salas) + 1) * sizeof(char), addr, addrlen);
        }
        free(id);
        free(titulo);
        free(sinopse);
        free(genero);
        free(salas);
    }
    free(linhas);
    fclose(fp);
}

/*
 * Funcao: escolheOperacao
 * -----------------------
 * Recebe do cliente a operacao escolhida e inicia o processamento. Servidor chama funcao que conta o numero
 * de linhas do catalogo de filmes, para tratar o caso de ainda não existirem filmes cadastrados. Se a lista de filmes
 * ainda não existir ou estiver vazia, a variavel status = 0, caso contrário status = 1. O status também é enviado
 * ao cliente para fazer a tratativa correspondente.
 *
 * sockfd: inteiro descritor do socket
 *
 */
void escolheOperacao(int sockfd, struct sockaddr* addr, int addrlen) {
    for ( ; ; ) {
        printf("\nRecebendo operacao do cliente\n");
        char* op = receber(sockfd, addr, &addrlen);
        printf("\nExecutando operacao %s\n", op);

        if (strcmp(op, "1") == 0) cadastrar(sockfd, addr, addrlen);
        else if (strcmp(op, "8") == 0) {
            //printf("Cliente encerrou conexao\n");
            exit(0);
        }
        else {
            char* status;
            int numFilmes = contaLinhas("listaFilmes");
            if (numFilmes == -1 || numFilmes == 0) {
                printf("\nO catalogo de filmes esta vazio, somente a operacao de cadastro pode ser realizada\n");
                status = "0";
                enviar(sockfd, status, (strlen(status) + 1) * sizeof(char), addr, addrlen);
            }
            else {
                status = "1";
                enviar(sockfd, status, (strlen(status) + 1) * sizeof(char), addr, addrlen);
                if (strcmp(op, "2") == 0) remover(sockfd, addr, addrlen);
                else if (strcmp(op, "3") == 0) getTituloSalas(sockfd, addr, addrlen);
                else if (strcmp(op, "4") == 0) getTituloGenero(sockfd, addr, addrlen);
                else if (strcmp(op, "5") == 0) getTitulo(sockfd, addr, addrlen);
                else if (strcmp(op, "6") == 0) getAll(sockfd, addr, addrlen);
                else getCatalogo(sockfd, addr, addrlen);
            }
        }
    }
}

int main(int argc, char** argv) {
    int    sockfd;
    struct sockaddr_in addr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERV_PORT);

    if (bind(sockfd, (struct sockaddr*) & addr, sizeof(addr)) == -1) {
        perror("bind");
        exit(1);
    }

    printf("Servidor aguardando conexoes\n");

    int addrlen = sizeof(addr);

    // Operações no catálogo de filmes
    escolheOperacao(sockfd, (struct sockaddr*) & addr, addrlen);

    close(sockfd);

    return(0);
}
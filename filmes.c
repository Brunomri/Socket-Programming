#pragma warning(disable : 4996)
#include <stdio.h>
#define maxLen 200

ssize_t readn(int fd, void *vptr, size_t n) {
	size_t nleft;
	ssize_t nread;
	char *ptr;

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

ssize_t writen(int fd, const void *vptr, size_t n) {
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

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

// Estrutura filme com título, sinopse, gênero, salas em exibição e identificador único
typedef struct filmes
{
	char* id;
	char* titulo;
	char* sinopse;
	char* genero;
	char* salas;
} filme;

// Cria um ID para ser atribuído a um novo filme
int criarID() {
	srand(time(0));
	int id = rand();
	//printf("novo id: %s", id);
	return id;
}

// TODO: Cadastrar novo filme e retornar identificador
int cadastrar(filme* novo) {
	int idTemp = criarID();
	char id[maxLen];
	sprintf(id, "%d", idTemp);
	novo->id = id;

	//char id[maxLen] = "id";
	//char extensao[] = ".txt";
	//strcat(&id, &extensao);

	FILE* fp;
	fp = fopen(novo->id, "w");
	fputs(novo->id, fp);
	fputs("\n", fp);
	fputs(novo->titulo, fp);
	fputs("\n", fp);
	fputs(novo->sinopse, fp);
	fputs("\n", fp);
	fputs(novo->genero, fp);
	fputs("\n", fp);
	fputs(novo->salas, fp);
	fclose(fp);

	printf("novo filme cadastrado:\n");
	printf("id: %s\n", novo->id);
	printf("titulo: %s\n", novo->titulo);
	printf("sinopse: %s\n", novo->sinopse);
	printf("genero: %s\n", novo->genero);
	printf("salas: %s\n", novo->salas);

	return novo->id;
}

// TODO: Remover um filme a partir do seu identificador

// TODO: Listar o título e salas de exibição de todos os filmes

// TODO: Listar todos os títulos de filmes de um determinado gênero

// TODO: Dado o identificador de um filme, retornar o título do filme

// TODO: Dado o identificador de um filme, retornar todas as informações deste filme

// TODO: Listar todas as informações de todos os filmes

int main() {
	
	printf("Gerenciamento de filmes\n\n");
	filme novo = {NULL, "1917","a","b","5,6,7"};
	cadastrar(&novo);
}
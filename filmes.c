#pragma warning(disable : 4996)
#include <stdio.h>
#define maxLen 200

// TODO: Criar estrutura filme com título, sinopse, gênero, salas em exibição e identificador único
typedef struct filmes
{
	int id;
	char titulo[maxLen];
	char sinopse[maxLen];
	char genero[maxLen];
	char salas[maxLen];
} filme;

// Cria um ID para ser atribuído a um novo filme
int criarID() {

}

// TODO: Cadastrar novo filme e retornar identificador
int cadastrar(filme* novo) {
	char id[maxLen] = "id";
	char extensao[] = ".txt";
	strcat(&id, &extensao);

	// TODO: Criar função geradora de id
	FILE* fp;
	fp = fopen(id, "w");
	fputs(&id, fp);
	fputs("\n", fp);
	fputs(novo->titulo, fp);
	fputs("\n", fp);
	fputs(novo->sinopse, fp);
	fputs("\n", fp);
	fputs(novo->genero, fp);
	fputs("\n", fp);
	fputs(novo->salas, fp);
	fclose(fp);

	printf("Novo filme cadastrado:\n");
	printf("id: %s\n", novo->id);
	printf("titulo: %s\n", novo->titulo);
	printf("sinopse: %s\n", novo->sinopse);
	printf("genero: %s\n", novo->genero);
	printf("salas: %s\n", novo->salas);

	return id;
}

// TODO: Remover um filme a partir do seu identificador

// TODO: Listar o título e salas de exibição de todos os filmes

// TODO: Listar todos os títulos de filmes de um determinado gênero

// TODO: Dado o identificador de um filme, retornar o título do filme

// TODO: Dado o identificador de um filme, retornar todas as informações deste filme

// TODO: Listar todas as informações de todos os filmes

int main() {
	
	printf("Gerenciamento de filmes\n");
	filme novo = {NULL, "1917","a","b","5,6,7"};
	cadastrar(&novo);
}
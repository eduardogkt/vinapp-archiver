/*
 * libAVL.h
 * biblioteca que define a estrutura de cabecalho
 * dos arquivos e funcoes de implemetacao da arvore
 * AVL e da lista.
 */

#ifndef _LIB_AVL_H_
#define _LIB_AVL_H_

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#define NIVEL_RAIZ 0
#define NAME_SIZE 100

typedef unsigned int uint_t;
typedef unsigned long ulong_t;

// estrutura do cabecalho de metadados
typedef struct cabecalho {
    char nome[NAME_SIZE];  // nome original
    char path[NAME_SIZE];  // path relativo
    uid_t uid;             // user id
    gid_t gid;             // group id
    mode_t perms;          // permissoes
    uint_t ordem;          // ordem de insercao
    time_t tmp_mod;        // tempo de modificacao
    size_t tamanho;        // tamanho do arquivo
    size_t ini_arq;        // localizacao do arquivo
} vina_cabecalho_t;

// estrutura do no do avl
typedef struct no {
    vina_cabecalho_t *cabecalho;  // infos do arquivo
    uint_t altura;                // altura do no na arvore
    uint_t nivel;                 // nivel do no na arvore
    struct no *esq, *dir, *pai;   // ponteiros da arvore avl
    struct no *ant, *prox;        // ponteiro para a lista
} no_t;

// estrutura para interpretar avl como
// lista, preservando ordem de insersão
typedef struct lista_avl {
    no_t *ini;             // ponteiro inicio da lista
    no_t *fim;             // ponteiro final da lista
    size_t tamanho;        // tamanho (quantidade de nos) da lista
    size_t soma_tam_arqs;  // soma do tamanho dos arquivos armazenados
} lista_avl_t;


////////// FUNCOES IMPLEMENTACAO DE ARVORE GERAL //////////

// cria um no e o retorna, caso falhe, retorna NULL
no_t *criaNo(vina_cabecalho_t cabecalho, int nivel);

// retorna o maior elemento da arvore de raiz em no
no_t *maxArvore(no_t *no);

// faz a busca pelo no com o nome especificada e o retorna
no_t *buscaNo(no_t *no, char *nome);

// insere o no com a chave especificada e o armazena em 'ins'
// retorna a raiz da arvore
no_t *insereBST(no_t *no, no_t **ins, lista_avl_t *lista, vina_cabecalho_t *cabecalho, int nivel);

// faz o transplante do no novo para o no, troca o ponteiro
// do pai e ajusta o nivel dos nos
void transplante(no_t *no, no_t *novo_no, no_t **raiz);

// remove o no com a chave especificada e o retorna
// faz o ajunte de nova raiz
no_t* removeBST(no_t *raiz, lista_avl_t *lista, char *nome, no_t **nova_raiz);

// desaloca a memoria da arvore
void destroiArvore(no_t *raiz);

// percorre a arvore em ordem imprimindo a chave e o nivel
void emOrdem(no_t *no);


////////// FUNCOES IMPLEMENTACAO AVL //////////

// retorna a altura da arvore
int altura(no_t *raiz);

// aumenta o nivel do no e de sua subarvore
void aumentaNivel(no_t *no);

// diminui o nivel do no e de sua subarvore
void diminuiNivel(no_t *no);

// faz a rotacao a esqueda do no x
// atualiza o nivel e altura dos nos 
no_t *rotEsquerda(no_t *x);

// faz a rotacao a direita do no x
// atualiza o nivel e altura dos nos 
no_t *rotDireita(no_t *x);

// retorna o fator de balanceamento do no
int fatorBalanceamento(no_t *no);

// ajusta o ponteiro de pai do no rotacionado
void arrumaPai(no_t *no, char *nome);

// faz as rotacoes para o caso esq esq
void casoEsqEsq(no_t *no, char *nome);

// faz as rotacoes para o caso esq dir
void casoEsqDir(no_t *no, char *nome);

// faz as rotacoes para o caso dir dir
void casoDirDir(no_t *no, char *nome);

// faz as rotacoes para o caso dir esq
void casoDirEsq(no_t *no, char *nome);

// faz o balanceamento do no
no_t *balanceia(no_t *no, char *nome);

// insere o no com a chave especificada e retorna a raiz
no_t *insereAVL(no_t *raiz, lista_avl_t *lista, vina_cabecalho_t *cabecalho);

// remove o no com a chave especificada e retorna a nova raiz
no_t *removeAVL(no_t *raiz, lista_avl_t *lista, char *nome);


////////// FUNCOES AVL INTERPRETADA COMO LISTA  //////////

// cria e retorna uma lista com inicio na raiz da arvore
lista_avl_t *criaListaAVL(no_t *raiz);

// desaloca a memoria utilizada pela lista
void listaAVLDestroi(lista_avl_t *lista);

// retorna 1 se a lista estiver vazia
int listaAVLVazia(lista_avl_t *lista);

// retorna o tamanho da lista
int listaAVLTamanho(lista_avl_t *lista);

// organiza os ponteiros do no da lista para insersao no fim da lista
void listaAVLInsere(lista_avl_t *lista, no_t *no);

// reorganiza os ponteiros do no da lista para remocao
void listaAVLRemove(lista_avl_t *lista, no_t *no);

// imprime a lista em ordem de insersao
void listaAVLImprime(lista_avl_t *lista);

#endif // _LIB_AVL_H_
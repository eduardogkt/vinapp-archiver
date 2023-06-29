#include "libAVL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////// FUNCOES IMPLEMENTACAO DE ARVORE GERAL //////////

// cria um no e o retorna, caso falhe, retorna NULL
no_t *criaNo(vina_cabecalho_t cabecalho, int nivel) {
    
    no_t *no = (no_t *) malloc(sizeof(no_t));
    if (no == NULL) return NULL;

    // alocando espaco para o cabecalho
    no->cabecalho = (vina_cabecalho_t *) malloc(sizeof(vina_cabecalho_t));
    if (!no->cabecalho) return NULL;
    memcpy(no->cabecalho, &cabecalho, sizeof(vina_cabecalho_t));

    no->altura = 0;
    no->nivel = nivel;
    no->dir = no->esq = no->pai = NULL;
    no->ant = no->prox = NULL;

    return no;
}

// retorna o maior elemento da arvore de raiz em no
no_t *maxArvore(no_t *no) {
    
    if (no->dir)
        return maxArvore(no->dir);
    return no;
}

// faz a busca pelo no com a chave especificada e o retorna
no_t *buscaNo(no_t *no, char *nome) {

    if (no == NULL) 
        return NULL;

    if (strcmp(nome, no->cabecalho->path) < 0)
        return buscaNo(no->esq, nome);
    else if (strcmp(nome, no->cabecalho->path) > 0)
        return buscaNo(no->dir, nome);
    else
        return no; // se igual, achou
}

// insere o no com a chave especificada e o armazena em 'ins'
// retorna a raiz da arvore
no_t *insereBST(no_t *no, no_t **ins, lista_avl_t *lista, vina_cabecalho_t *cabecalho, int nivel) {
    
    if (no == NULL) {
        no_t *novo_no = criaNo(*cabecalho, nivel);
        listaAVLInsere(lista, novo_no);
        *ins = novo_no;
        return novo_no;
    }

    if (strcmp(cabecalho->path, no->cabecalho->path) < 0) {
        nivel++;
        no->esq = insereBST(no->esq, ins, lista, cabecalho, nivel);
        no->esq->pai = no;
    } 
    else if (strcmp(cabecalho->path, no->cabecalho->path) >= 0) {
        nivel++;
        no->dir = insereBST(no->dir, ins, lista, cabecalho, nivel);
        no->dir->pai = no;
    }
    return no;
}

// faz o transplante do no novo para o no, troca o ponteiro
// do pai e ajusta o nivel dos nos
void transplante(no_t *no, no_t *novo, no_t **raiz) { 
    
    if (no != NULL && novo != NULL)
        novo->nivel = no->nivel;

    if (no->pai == NULL) // n eh raiz
        *raiz = novo;
    else if (no->pai->esq == no) // filho da esq
        no->pai->esq = novo;
    else if (no->pai->dir == no) // filho da dir
        no->pai->dir = novo;

    if (novo != NULL) // torna o pai de n o pai de novo
        novo->pai = no->pai;
}

// remove o no com a chave especificada e o retorna
// faz o ajunte de nova raiz
no_t* removeBST(no_t *raiz, lista_avl_t *lista, char *nome, no_t **nova_raiz) {
    
    no_t *rem, *ant;
    *nova_raiz = raiz;
    no_t *no = buscaNo(raiz, nome);
    listaAVLRemove(lista, no);

    // no nao encontrado na arvore
    if (no == NULL) {
        return raiz;
    }
    
    // caso 1 ou 2 (nenhum filho ou 1 filho dir)
    if (no->esq == NULL) {
        transplante(no, no->dir, nova_raiz);
        rem = no->pai;
        free(no->cabecalho);
        free(no);
    } 
    // caso 2 (um filho esq)
    else if (no->dir == NULL) { 
        transplante(no, no->esq, nova_raiz);
        rem = no->pai;
        free(no->cabecalho);
        free(no);
    }
    // caso 3 (dois filhos)
    else { 
        ant = maxArvore(no->esq); // antecessor
        if (ant != no->esq) {
            transplante(ant, ant->esq, nova_raiz);
            ant->esq = no->esq;
            ant->esq->pai = ant;
        }
        transplante(no, ant, nova_raiz);
        ant->dir = no->dir;
        ant->dir->pai = ant;
        rem = ant; // rem aponta para o no substituto
        free(no->cabecalho);
        free(no);
    }
    return rem;
}

// desaloca a memoria da arvore
void destroiArvore(no_t *no) {

    if (no != NULL) {
        destroiArvore(no->dir);
        destroiArvore(no->esq);
        free(no->cabecalho);
        free(no);
    }
}

// percorre a arvore em ordem imprimindo a chave e o nivel
void emOrdem(no_t *no) {

    if (no != NULL) {
        emOrdem(no->esq);
        printf("%s,%d\n", no->cabecalho->path, no->nivel);
        emOrdem(no->dir);
    }
}

////////// FUNCOES IMPLEMENTACAO AVL //////////

// retorna o maior entre dois inteiros
int max(int a, int b) {

    return (a > b)? a : b;
}

// retorna a altura da arvore
int altura(no_t *no) {

    int he, hd;

    if (no == NULL)
        return -1; // se anula com o +1

    he = altura(no->esq);
    hd = altura(no->dir);

    return max(he, hd) + 1;
}

// aumenta o nivel do no e de sua subarvore
void aumentaNivel(no_t *no) {

    no->nivel++;
    if (no->esq != NULL)
        aumentaNivel(no->esq);
    
    if (no->dir != NULL)
        aumentaNivel(no->dir);
}

// diminui o nivel do no e de sua subarvore
void diminuiNivel(no_t *no) {

    no->nivel--;
    if (no->esq != NULL)
        diminuiNivel(no->esq);
    
    if (no->dir != NULL) 
        diminuiNivel(no->dir);
}

// faz a rotacao a esqueda do no x
// atualiza o nivel e altura dos nos 
no_t *rotEsquerda(no_t *x) {
    
    no_t *y = x->dir;
    if (!x->dir) {
        fprintf(stderr, "Erro específico nao resolvido (abortando).\n"); 
        exit(1);
    }
    x->dir = y->esq;
    y->pai = x->pai;
    x->pai = y;
    if (y->esq != NULL) 
        y->esq->pai = x;
    y->esq = x;

    // atualiza altura
    y->altura = altura(y);
    x->altura = altura(x);

    // autualiza nivel
    y->nivel--;
    if(y->dir != NULL)
        diminuiNivel(y->dir);
    x->nivel++;
    if(x->esq != NULL)
        aumentaNivel(x->esq);

    return y;
}

// faz a rotacao a direita do no x
// atualiza o nivel e altura dos nos 
no_t *rotDireita(no_t *x) {
    
    no_t *y = x->esq;

    x->esq = y->dir;
    y->pai = x->pai;
    x->pai = y;
    if (y->dir != NULL) 
        y->dir->pai = x;
    y->dir = x;

    // atualiza altura
    y->altura = altura(y);
    x->altura = altura(x);

    // atualiza nivel
    y->nivel--;
    if(y->esq != NULL)
        diminuiNivel(y->esq);
    x->nivel++;
    if(x->dir != NULL)
        aumentaNivel(x->dir);

    return y;
}

// retorna o fator de balanceamento do no
int fatorBalanceamento(no_t *no) {

    if (no == NULL)
        return 0;
    return (altura(no->esq) - altura(no->dir));
}

// ajusta o ponteiro de pai do no rotacionado
void arrumaPai(no_t *no, char *nome) {

    // no inserido a esquerda, filho da esquerda
    if (no->pai != NULL && strcmp(nome, no->pai->cabecalho->path) < 0)
        no->pai->esq = no;
    // no inserido a direita, filho da direita
    if (no->pai != NULL && strcmp(nome, no->pai->cabecalho->path) >= 0)
        no->pai->dir = no; 
}

// faz as rotacoes para o caso esq esq
void casoEsqEsq(no_t *no, char *nome) {

    no = rotDireita(no);
    arrumaPai(no, nome);
}

// faz as rotacoes para o caso esq dir
void casoEsqDir(no_t *no, char *nome) {

    no->esq = rotEsquerda(no->esq);
    no = rotDireita(no);
    arrumaPai(no, nome);
}

// faz as rotacoes para o caso dir dir
void casoDirDir(no_t *no, char *nome) {

    no = rotEsquerda(no);
    arrumaPai(no, nome);
}

// faz as rotacoes para o caso dir esq
void casoDirEsq(no_t *no, char *nome) {

    no->dir = rotDireita(no->dir);
    no = rotEsquerda(no);
    arrumaPai(no, nome);
}

// faz o balanceamento do no
no_t *balanceia(no_t *no, char *nome) { 

    int fb = fatorBalanceamento(no);

    if (fb > 1 && fatorBalanceamento(no->esq) >= 0) 
        casoEsqEsq(no, nome);
    else
    if (fb < -1 && fatorBalanceamento(no->dir) <= 0) 
        casoDirDir(no, nome);
    else
    if (fb > 1 && fatorBalanceamento(no->esq) < 0) 
        casoEsqDir(no, nome);
    else
    if (fb < -1 && fatorBalanceamento(no->dir) > 0) 
        casoDirEsq(no, nome);
    
    return no;
}

// insere o no com a chave especificada e retorna a raiz
no_t *insereAVL(no_t *raiz, lista_avl_t *lista, vina_cabecalho_t *cabecalho) {

    // faz a insercao BST e retorna o no inserido
    no_t *no;
    raiz = insereBST(raiz, &no, lista, cabecalho, NIVEL_RAIZ);
    no_t *x = no->pai;

    // percorre a arvore da folha para a raiz rebalanceando
    while (x != NULL) {
        x->altura = altura(x);
        x = balanceia(x, cabecalho->path);
        no = x;
        x = x->pai;
    }
    return no; // nova raiz
}

// remove o no com a chave especificada e retorna a nova raiz
no_t *removeAVL(no_t *raiz, lista_avl_t *lista, char *nome) {

    // faz a remocao BST e retorna o pai do no removido
    no_t *nova_raiz = raiz;
    no_t *rem = removeBST(raiz, lista, nome, &nova_raiz);
    no_t *x = rem;

    // percorre a arvore da folha para a raiz rebalanceando
    while (x != NULL) {
        x->altura = altura(x);
        x = balanceia(x, nome);
        if (x->pai == NULL) // caso o balanceamento mude a raiz
            nova_raiz = x;
        x = x->pai;
    }
    return nova_raiz;
}

////////// FUNCOES AVL INTERPRETADA COMO LISTA  //////////

// cria e retorna uma lista com inicio na raiz da arvore
lista_avl_t *criaListaAVL(no_t *raiz) {

    lista_avl_t *lista = (lista_avl_t *) malloc(sizeof(lista_avl_t));
    if (!lista) return NULL;

    lista->ini = raiz;
    lista->fim = NULL;
    lista->tamanho = 0;
    lista->soma_tam_arqs = 0;

    return lista;
}

// desaloca a memoria utilizada pela lista
void listaAVLDestroi(lista_avl_t *lista) {

    while (!listaAVLVazia(lista)) {
        no_t *no = lista->ini;
        lista->ini = lista->ini->prox;
        free(no->cabecalho);
        free(no);
        lista->tamanho--;
    }
    free(lista);
}

// retorna 1 se a lista estiver vazia
int listaAVLVazia(lista_avl_t *lista) {

    return (lista->tamanho == 0);
}

// retorna o tamanho da lista
int listaAVLTamanho(lista_avl_t *lista) {

    return lista->tamanho;
}

// organiza os ponteiros do no da lista para insercao no fim da lista
void listaAVLInsere(lista_avl_t *lista, no_t *no) {

    if (lista->ini == NULL) {
        lista->ini = no;
        lista->fim = no;
    }
    else {
        lista->fim->prox = no;
        no->ant = lista->fim;
        lista->fim = no;
    }
    lista->tamanho++;
    lista->soma_tam_arqs += no->cabecalho->tamanho;
}

// reorganiza os ponteiros do no da lista para remocao
void listaAVLRemove(lista_avl_t *lista, no_t *no) {

    if (listaAVLVazia(lista) || no == NULL)
        return;
    
    // primeiro elemento da lista
    if (no->ant == NULL) {
        lista->ini = no->prox;
    }
    // ultimo elemento da lista
    if (no->prox == NULL) {
        lista->fim = no->ant;
    }
    // ajustando ponteriros ant e prox
    if (no->ant != NULL) {
        no->ant->prox = no->prox;
    }
    if (no->prox != NULL) {
        no->prox->ant = no->ant;
    }
    lista->tamanho--;
    lista->soma_tam_arqs -= no->cabecalho->tamanho;
}

// imprime a lista em ordem de insersao
void listaAVLImprime(lista_avl_t *lista) {

    if (listaAVLVazia(lista))
        return;

    no_t *no = lista->ini;
    while (no->prox != NULL) {
        printf("%s ", no->cabecalho->path);
        no = no->prox;
    }
    printf("%s\n", no->cabecalho->path);
}
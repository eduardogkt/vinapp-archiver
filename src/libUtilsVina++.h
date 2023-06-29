/*
 * libUtilsVina++.h
 * biblioteca que define as constante e as funcoes
 * auxilizares das funcoes de opcao vina.
 */

#ifndef _LIB_UTILS_VINAPP_H_
#define _LIB_UTILS_VINAPP_H_

#include <stdio.h>
#include "libAVL.h"

#define TRUE 1
#define FALSE 0

#define __USE_XOPEN_EXTENDED  // utilizado para a funcao truncate

#define BUFFER_SIZE 1024  // buffer de 1024 bytes

// arquivo temporario usado para tranferir conteudo
#define VINA_TMP_FILE "vina_tmp_file"
#define VINA_TMP_MOVE_FILE "vina_tmp_move_file"

// comando para remover arquivo temporario
#define VINA_RM_TMP_FILE "rm vina_tmp_file"       
#define VINA_RM_TMP_MOVE_FILE "rm vina_tmp_move_file"

// constantes que indicam se os arquivos devem ser atualizados caso mais recente
#define VINA_UPDATE_ALWAYS 0  // sempre atualiza o conteudo do arquivo
#define VINA_UPDATE_RECENT 1  // atualiza apenas se mais recente

// constantes utilizadas para verificar qual arquivo eh mais recente para opcao -a
#define VINA_ARQ_INTERNO 0  // indica o arquivo que ja existe no arquivo vina
#define VINA_ARQ_EXTERNO 1  // indica o arquivo a ser inserido no arquivo vina

// constantes de opcao
#define VINA_OPTION_I 'i'  // opcao inserir
#define VINA_OPTION_A 'a'  // opcao atualizar
#define VINA_OPTION_M 'm'  // opcao mover
#define VINA_OPTION_X 'x'  // opcao extrair
#define VINA_OPTION_R 'r'  // opcao remover
#define VINA_OPTION_C 'c'  // opcao listar
#define VINA_OPTION_H 'h'  // opcao ajuda

// constantes de erro
#define VINA_ERROR_ENTRY -1    // erro ao ler entrada
#define VINA_ERROR_OPEN -2     // erro ao abrir arquivo vina
#define VINA_ERROR_INSERT -3   // erro ao inserir arquivos
#define VINA_ERROR_UPDATE -4   // erro ao atualizar arquivos
#define VINA_ERROR_MOVE -5     // erro ao mover arquivos
#define VINA_ERROR_EXTRACT -6  // erro ao extrair arquivos
#define VINA_ERROR_REMOVE -7   // erro ao remover arquivos
#define VINA_ERROR_LIST -8     // erro ao listar arquivos
#define VINA_ERROR_FOPEN -9    // erro ao abrir arquivo

// verifica se o arquivo vina tem a extensao correta .vpp
int extensaoVina(char *vina_path);

// retorna verdadeiro caso o arquivo exista
int arquivoExiste(char *filepath);

// padroniza o filename para que sejam arquivados como caminha relativo
char *caminhoRelativo(char *filepath);

// funcao auxilizar que imprime os metadados de um arquivo
void imprimeCabecalho(vina_cabecalho_t *cabecalho);

// imprime todos os cabecalhos dos arquivos na arvore em ordem
void imprimeCabecalhos(lista_avl_t *lista);

// preenche o cabecalho com os metadados do arquivo
int preencheCabecalho(vina_cabecalho_t *cabecalho, char *filepath, ulong_t ini_arq, uint_t ordem);

// insere os cabecalhos do arquivo vina na arvore AVL
int coletaCabecalhos(FILE *vina_file, no_t **raiz, lista_avl_t *lista, uint_t *ordem, ulong_t ini_dir);

// padroniza o filepath para o caminho realitivo e 
// procura o cabecalho correspontente
no_t *achaCabecalho(no_t *raiz, char *filepath);

// atualiza informacoes do no com as informaçoes do cabecalho
void atualizaCabecalho(vina_cabecalho_t cabecalho, no_t *no);

// realiza a transferencia do conteudo de um arquivo de nome filename
// para o arquivo vina
int transfereParaVina(FILE *vina_file, char *filename);

// transfere para o arquivo de nome filename o conteudo do arquivo 
// vina de tamanho tam_arq
int transfereParaArquivo(FILE *vina_file, char *filename, ulong_t tam_arq);

// desloca o conteudo de um arquivo 'deslocamento' bytes para frente
// considera que o long deslocamento passado en positivo
int deslocaParaFrente(FILE *vina_file, no_t *no, long deslocamento);

// desloca conteudo do arquivo 'deslocamento' bytes para tras
// considera que o long deslocamento passado eh negetivo
int deslocaParaTras(FILE *vina_file, no_t *no, long deslocamento);

// atualiza a localizacao do conteudo de todos os arquivos subsequentes
// ao arquivo substituido
int atualizaArquivosSubsequentes(FILE *vina_file, lista_avl_t *lista, no_t *no_encontrado, long deslocamento);

// retorna VINA_ARQ_EXTERNO caso o arquivo a ser inserido for mais 
// recente e retorna VINA_ARQ_INTERNO caso o arquivo que ja esta 
// no archive for mais recente
int arquivoMaisRecente(vina_cabecalho_t *externo, vina_cabecalho_t *interno);

// atualiza o inteiro que indica o inicio do diretorio
// e grava os cabecalhos no arquivo vina
int atualizaDiretorio(FILE *vina_file, lista_avl_t *lista);

// calcula o novo tamanho do arquivo vina e trunca-o
void truncaArquivoVina(char *vina_path, lista_avl_t *lista);

// ajusta os ponteiros do no target e do no a ser movido
void atualizaPonteirosNos(lista_avl_t *lista, no_t *no_target, no_t *no_movido);

// verificar casos nos quais nao eh preciso fazer movimentacoes
int atendeRequisitosMover(no_t *no_movido, no_t *no_target);

// insere o arquivo a ser movido imediatamente depois do target
int insereArquivoMovido(FILE *vina_file, lista_avl_t *lista, no_t *no_target, no_t *no_movido);

// remove o arquivo a ser movido da área de dados dos arquivos
int removeArquivoMovido(FILE *vina_file, lista_avl_t *lista, no_t *no_target, no_t *no_movido);

// recria a hierarquia de diretorios no diretorio atual, caso nao exista
void criaDiretorios(char *filename);

// extrai todos o arquivo indicado por filepath
// caso filepath seja NULL, extrai todos arquivos
void extraiArquivos(FILE *vina_file, ulong_t ini_dir, char *filepath);

// imprime os matadados do arquivo indicado pelo cabecalho
void imprimeAtributos(vina_cabecalho_t cabecalho);

#endif // _LIB_UTILS_VINAPP_H_
/*
 * libVina++.h
 * biblioteca que define as funcoes de opcao vina.
 */

#ifndef _LIB_VINAPP_H_
#define _LIB_VINAPP_H_

#include <stdio.h>
#include "libAVL.h"
#include "libUtilsVina++.h"

// le e verifica parametros de entrada
void vinaLeEntrada(int argc, char **argv, char *opcao, char **vina_path, char **target_path);

// abre e, caso nao exista e for opcao de inserir, padroniza o arquivo
// vina colocando um int no inicio para para indicar o comeco da area de
// diretorio
FILE *vinaAbreArchive(char *vina_path, int opcao);

// retorna verdadeiro se o arquivo vina estiver vazio, isto ocorre 
// quando o inicio do diretorio for logo apos o inteiro que indica 
// a sua localizacao
int vinaVazio(ulong_t ini_dir);

// insere/acrescenta os membros indicados ao archive
// novos membros são inseridos respeitando a ordem da linha de comando
// a flag atualiza_recente indica se o arquivo deve ser atualizado caso 
// este ja exista no archive e for mais recente (opcao -a)
void vinaInsere(char *vina_path, int argc, char **argv, int atualiza_recente);

// remove os membros indicados de archive
void vinaRemove(char *vina_path, int argc, char **argv);

// move o membro indicado na linha de comando para imediatamente depois do 
// membro target existente no arquivo vina
void vinaMove(char *vina_path, char *target_path, int argc, char **argv);

// extrai os membros indicados de archive
// se os membros não forem indicados, todos sao extraidos
void vinaExtrai(char *vina_path, int argc, char **argv);

// lista o conteudo do arquivo vina, saida na forma:
// permissoes  usuario/grupo  tamanho  data_modificacao  nome
void vinaLista(char *vina_path, int argc, char **argv);

// imprime mensagem de ajuda com as opções disponíveis e encerra
void vinaAjuda();

// fecha o arquivo vina, desaloca a lista de arquivos e aborta 
// o programa com VINA_ERROR_CODE
void vinaAborta(FILE *vina_file, lista_avl_t *lista, int VINA_ERROR_CODE);

#endif // _LIB_VINAPP_H_

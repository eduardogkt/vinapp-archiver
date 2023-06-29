#include "libVina++.h"
#include "libUtilsVina++.h"
#include "libAVL.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

// le e verifica parametros de entrada
void vinaLeEntrada(int argc, char **argv, char *opcao, char **vina_path, char **target_path) {
    
    // verificando quantidade de opcoes usadas
    int quant_opcoes = 0;
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-')
            quant_opcoes++;
    }
    if (quant_opcoes > 1) {
        fprintf(stderr, "Erro: opcoes demais.\n");
        exit(VINA_ERROR_ENTRY);
    }

    opterr = FALSE; // desabilitando mensagens de erro do getopt
    *opcao = getopt(argc, argv, "i:a:m:x:r:c:h");
    switch (*opcao) {
        case 'i': case 'a': case 'x': case 'r': case 'c':
            *target_path = NULL;
            *vina_path = optarg;
            if (!*vina_path) {
                fprintf(stderr, "Erro: arquivo vina nao especificado.\n");
                exit(VINA_ERROR_ENTRY);
            }
            break;
        case 'm':
            *target_path = optarg;
            *vina_path = argv[3];
            if (!*vina_path || !*target_path) {
                fprintf(stderr, "Erro: arquivo vina ou target nao especificado.\n");
                exit(VINA_ERROR_ENTRY);
            }
            break;
        case 'h':
            break;
        default:
            fprintf(stderr, "Erro: entrada invalida.\n");
		    exit(VINA_ERROR_ENTRY);
    }
    // verificando se o arquivo vina tem a extensao .vpp
    if (!extensaoVina(*vina_path)) {
        fprintf(stderr, "Erro: extensao de arquivo vina invalido, use um arquivo \".vpp\".\n");
        exit(VINA_ERROR_ENTRY);
    }
}

// abre e, caso nao exista, padroniza o arquivo vina colocando um
// int no inicio para para indicar o comeco da area de diretorio
FILE *vinaAbreArchive(char *vina_path, int opcao) {

    FILE *vina_file = NULL;

    if (arquivoExiste(vina_path)) {
        if (!(vina_file = fopen(vina_path, "r+"))) {
            fprintf(stderr, "Erro: nao foi possivel abrir o arquivo vina.\n");
            return NULL;
        }
    }
    // cria um archive novo apenas se for opcao insere
    else if (opcao == VINA_OPTION_I) {
        if (!(vina_file = fopen(vina_path, "w+"))) {
            fprintf(stderr, "Erro: nao foi possivel abrir o arquivo vina.\n");
            return NULL;
        }
        // padroniza o novo archive para que tenha um inteiro
        // no inicio indicando o comeco da area de diretorio
        ulong_t ini_dir = sizeof(ulong_t);
        ulong_t escrito = fwrite(&ini_dir, sizeof(ulong_t), 1, vina_file);
        if (escrito != 1) {
            fprintf(stderr, "Erro: nao foi possivel criar o arquivo vina.\n");
            fclose(vina_file);
            return NULL;
        }
        fseek(vina_file, 0, SEEK_SET);
    }
    else {
        fprintf(stderr, "Erro: arquivo vina precisa existir para opcao \"-%c\".\n", opcao);
        return NULL;
    }
    return vina_file;
}

// retorna verdadeiro se o arquivo vina estiver vazio
// isto ocorre quando o inicio do diretorio for logo
// apos o inteiro que indica a sua localizacao
int vinaVazio(ulong_t ini_dir) {

    return (ini_dir == sizeof(ulong_t));
}

// insere/acrescenta um ou mais membros ao archive
// novos membros sao inseridos respeitando a ordem da linha de comando
// a flag atualiza_recente indica se o arquivo deve ser atualizado caso 
// este ja exista no archive e for mais recente (opcao -a)
void vinaInsere(char *vina_path, int argc, char **argv, int atualiza_recente) {

    // abre o arquivo vina adequadamente
    FILE *vina_file = vinaAbreArchive(vina_path, VINA_OPTION_I);
    if (!vina_file) 
        exit(VINA_ERROR_OPEN);

    no_t *raiz = NULL;
    lista_avl_t *lista = criaListaAVL(raiz);
    vina_cabecalho_t cabecalho;
    ulong_t ini_dir, ini_arq = sizeof(ulong_t);
    uint_t ordem = 0;

    // pegando o comeco do diretorio
    fread(&ini_dir, sizeof(ulong_t), 1, vina_file);

    // se arquivo vina nao esta vazio, os cabecalhos sao inseridos na arvore
    if (!vinaVazio(ini_dir)) {
        if (!coletaCabecalhos(vina_file, &raiz, lista, &ordem, ini_dir)) {
            vinaAborta(vina_file, lista, VINA_ERROR_INSERT);
        }
        // atualiza posicao de inicio de insercao de arquivo
        ini_arq += lista->soma_tam_arqs;
    }

    // inserindo novos arquivos ao arquivo vina
    for (int i = 3; i < argc; i++) {

        // em caso de erro ao preencher o cabecalho, passa para o proximo arquivo
        if (!preencheCabecalho(&cabecalho, argv[i], ini_arq, ordem))
            continue;

        // procura arquivo na arvore
        no_t *no_encontrado = buscaNo(raiz, cabecalho.path);

        // arquivo nao encontrado, novo arquivo a ser inserido
        if (no_encontrado == NULL) {
            // adicionando o cabecalho na arvore
            raiz = insereAVL(raiz, lista, &cabecalho);

            // adicionando o conteudo do arquivo no archive
            // em caso de erro, passa para o proximo arquivo
            fseek(vina_file, ini_arq, SEEK_SET);
            if (!transfereParaVina(vina_file, cabecalho.nome))
                continue;
            
            ini_arq += cabecalho.tamanho;
            ordem++;
        }
        // arquivo encontrado, ja existe no arquivo vina
        else {
            // verifica se a flag para atualizar esta ligada e qual arquivo eh mais recente.
            // se o arquivo encontrado (que ja esta no archive) for mais recente, nao atualiza
            if (atualiza_recente) {
                if (arquivoMaisRecente(&cabecalho, no_encontrado->cabecalho) == VINA_ARQ_INTERNO)
                    continue; // passa para o proximo arquivo sem atualizar
            }

            ulong_t tam_antigo = no_encontrado->cabecalho->tamanho;
            ulong_t tam_novo = cabecalho.tamanho;
            long dif = tam_novo - tam_antigo;

            // atualiza a localizacao do arquivos subsequentes ao arquivo a ser atualizado
            if (!atualizaArquivosSubsequentes(vina_file, lista, no_encontrado, dif)) 
                vinaAborta(vina_file, lista, VINA_ERROR_INSERT);
            
            // atualiza o tamanho total dos arquivos e cabecalho do no encontrado
            lista->soma_tam_arqs += dif;
            atualizaCabecalho(cabecalho, no_encontrado);

            // substituindo o conteudo do arquivo antigo
            fseek(vina_file, no_encontrado->cabecalho->ini_arq, SEEK_SET);
            if (!transfereParaVina(vina_file, cabecalho.nome))
                vinaAborta(vina_file, lista, VINA_ERROR_INSERT);
        }
    } // fim for
    // atualizando a area de diretorio
    if (!atualizaDiretorio(vina_file, lista))
        vinaAborta(vina_file, lista, VINA_ERROR_INSERT);
    
    truncaArquivoVina(vina_path, lista);
    
    fclose(vina_file);
    listaAVLDestroi(lista);
}

// remove os membros indicados de archive
void vinaRemove(char *vina_path, int argc, char **argv) {

    // abre o arquivo vina adequadamente
    FILE *vina_file = vinaAbreArchive(vina_path, VINA_OPTION_R);
    if (!vina_file) 
        exit(VINA_ERROR_OPEN);

    no_t *raiz = NULL;
    lista_avl_t *lista = criaListaAVL(raiz);
    ulong_t ini_dir, ini_arq = sizeof(ulong_t);
    uint_t ordem = 0;

    // pegando o comeco do diretorio
    fread(&ini_dir, sizeof(ulong_t), 1, vina_file);

    // se arquivo vina esta vazio, retorna erro
    if (vinaVazio(ini_dir)) {
        printf("Erro: archive esta vazio, nao ha nada para remover.\n");
        vinaAborta(vina_file, lista, VINA_ERROR_REMOVE);
    }

    if (!coletaCabecalhos(vina_file, &raiz, lista, &ordem, ini_dir)) {
        vinaAborta(vina_file, lista, VINA_ERROR_REMOVE);
    }
    ini_arq += lista->soma_tam_arqs;

    // removendo os arquivos do arquivo vina
    for (int i = 3; i < argc; i++) {

        // obtem caminho relativo arquivo
        char *filename = caminhoRelativo(argv[i]);

        // procura arquivo na arvore
        no_t *no_encontrado = buscaNo(raiz, filename);

        // arquivo nao encontrado
        if (no_encontrado == NULL) {
            fprintf(stderr, "Erro: \"%s\" nao encontrado.\n", filename);
            continue; // passa para o proximo arquivo
        }
        // arquivo encontrado, remover arquivo
        else {
            ulong_t tam_removido = no_encontrado->cabecalho->tamanho;
            no_t *no = no_encontrado->prox;
            while (no != NULL) {
                // desloca o conteudo do arquivo para tras para sobreescrever
                if (!deslocaParaTras(vina_file, no, -tam_removido))
                    vinaAborta(vina_file, lista, VINA_ERROR_REMOVE);

                // atualizando dados do arquivo
                no->cabecalho->ordem--;
                no->cabecalho->ini_arq -= tam_removido;

                no = no->prox;
            }

            // remove no da arvore
            raiz = removeAVL(raiz, lista, filename);
        }
        free(filename);
    }
    // atualizando a area de diretorio
    if (!atualizaDiretorio(vina_file, lista)) {
        vinaAborta(vina_file, lista, VINA_ERROR_REMOVE);
    }
    // diminuindo o tamanho do arquivo vina
    truncaArquivoVina(vina_path, lista);
    
    fclose(vina_file);
    listaAVLDestroi(lista);
}

// move o membro indicado na linha de comando para imediatamente depois do 
// membro target existente no arquivo vina
void vinaMove(char *vina_path, char *target_path, int argc, char **argv) {
    
    // abre o arquivo vina adequadamente
    FILE *vina_file = vinaAbreArchive(vina_path, VINA_OPTION_M);
    if (!vina_file) 
        exit(VINA_ERROR_OPEN);

    no_t *raiz = NULL;
    lista_avl_t *lista = criaListaAVL(raiz);
    ulong_t ini_dir, ini_arq = sizeof(ulong_t);
    uint_t ordem = 0;

    // pegando o comeco do diretorio
    fread(&ini_dir, sizeof(ulong_t), 1, vina_file);

    // se arquivo vina esta vazio, retorna erro
    if (vinaVazio(ini_dir)) {
        printf("Erro: archive esta vazio, nao ha nada para mover.\n");
        vinaAborta(vina_file, lista, VINA_ERROR_MOVE);
    }    

    if (!coletaCabecalhos(vina_file, &raiz, lista, &ordem, ini_dir)) {
        vinaAborta(vina_file, lista, VINA_ERROR_MOVE);
    }
    ini_arq += lista->soma_tam_arqs;

    // procuarando o arquivos necessarios na arvore
    no_t *no_target = achaCabecalho(raiz, target_path);
    no_t *no_movido = achaCabecalho(raiz, argv[4]);

    // verifica casos nos quais nao eh preciso fazer movimentacoes
    if (!atendeRequisitosMover(no_movido, no_target))
        vinaAborta(vina_file, lista, VINA_ERROR_MOVE);
    
    ulong_t ini_arq_movido = no_movido->cabecalho->ini_arq;
    ulong_t tam_arq_movido = no_movido->cabecalho->tamanho;

    // copiar o conteudo do movido para um arquivo temporario
    fseek(vina_file, ini_arq_movido, SEEK_SET);
    if (!transfereParaArquivo(vina_file, VINA_TMP_MOVE_FILE, tam_arq_movido))
        vinaAborta(vina_file, lista, VINA_ERROR_MOVE);

    // remove o arquivo a ser movido da area de dados do archive
    if (!removeArquivoMovido(vina_file, lista, no_target, no_movido))
        vinaAborta(vina_file, lista, VINA_ERROR_MOVE);

    // insere o arquivo movido depois do target
    if (!insereArquivoMovido(vina_file, lista, no_target, no_movido))
        vinaAborta(vina_file, lista, VINA_ERROR_MOVE);

    // reorganiza os ponteiros da lista 
    // e atualiza os metadados dos arquivos apos o target
    atualizaPonteirosNos(lista, no_target, no_movido);

    if (!atualizaDiretorio(vina_file, lista))
        vinaAborta(vina_file, lista, VINA_ERROR_MOVE);
    
    // trunca o arquivo com o tamanho correto para evitar erros
    truncaArquivoVina(vina_path, lista);

    fclose(vina_file);
    listaAVLDestroi(lista);
}

// extrai os membros indicados de archive
// se os membros nao forem indicados, todos sao extraidos
void vinaExtrai(char *vina_path, int argc, char **argv) {

    // abre o arquivo vina adequadamente
    FILE *vina_file = vinaAbreArchive(vina_path, VINA_OPTION_X);
    if (!vina_file) 
        exit(VINA_ERROR_OPEN);
    
    // pegando o comeco do diretorio
    ulong_t ini_dir;
    fread(&ini_dir, sizeof(ulong_t), 1, vina_file);

    if (vinaVazio(ini_dir)) {
        fprintf(stderr, "Erro: archive esta vazio, nao ha nada para extrair.\n");
        vinaAborta(vina_file, NULL, VINA_ERROR_EXTRACT);
    }

    // caso parametros nao seja fornecidos, extrai todos os arquivos
    if (argc < 4) {
        extraiArquivos(vina_file, ini_dir, NULL);
    }
    // caso parametros sejam fornecidos, extrai-os
    else {
        for (int i = 3; i < argc; i++)
            extraiArquivos(vina_file, ini_dir, argv[i]);
    }
}

// lista o conteudo do arquivo vina, saida na forma:
// permissoes  usuario/grupo  tamanho  data_modificacao  nome
void vinaLista(char *vina_path, int argc, char **argv) {

    // abre o arquivo vina adequadamente
    FILE *vina_file = vinaAbreArchive(vina_path, VINA_OPTION_C);
    if (!vina_file) 
        exit(VINA_ERROR_OPEN);
    
    ulong_t ini_dir;
    ulong_t lido;
    vina_cabecalho_t cabecalho;

    // pegando o comeco do diretorio
    fread(&ini_dir, sizeof(ulong_t), 1, vina_file);
    if (vinaVazio(ini_dir)) {
        fprintf(stdout, "Arquivo vina vazio.\n");
        return;
    }

    fseek(vina_file, ini_dir, SEEK_SET);
    lido = fread(&cabecalho,  sizeof(vina_cabecalho_t), 1, vina_file);
    while (!feof(vina_file)) {
        if (lido != 1) {
            fprintf(stderr, "Erro: falha ao listar arquivo \"%s\"", cabecalho.path);
            continue; // passa para o proximo arquivo
        }
        // imprime atibutos na forma
        // permissoes usr/grp tamanho data nome
        imprimeAtributos(cabecalho);

        lido = fread(&cabecalho, sizeof(vina_cabecalho_t), 1, vina_file);
    }
    fclose(vina_file);
}

// imprime mensagem de ajuda com as opcoes disponiveis e encerra
void vinaAjuda() {
    
    printf("Uso: ./vina++ <opcao> <archive> [membro1 membro2 ...]\n\n");
    printf("Descricao: vina++ salva varios arquivos em um unico archive de formado '.vpp'\n\n");
    printf("Opcoes:\n\n");
    printf(" -i          Insere/acrescenta um ou mais membros ao archive. Caso o membro ja\n");
    printf("             exista no archive, ele sera substituido. Novos membros sao\n");
    printf("             inseridos respeitando a ordem da linha de comando.\n\n");
    printf(" -a          Mesmo comportamento da opcao -i, mas a substituicao de um membro\n");
    printf("             existente ocorre APENAS caso o parâmetro seja mais recente que\n");
    printf("             arquivado.\n\n");
    printf(" -m <target> Move o membro indicado na linha de comando para imediatamente\n");
    printf("             depois do membro target existente em archive\n\n");
    printf(" -x          Extrai os membros indicados de archive. Se os membros nao forem\n");
    printf("             indicados, todos serao extraidos.\n\n");
    printf(" -r          Remove os membros indicados de archive.\n\n");
    printf(" -c          Lista o conteudo de archive em ordem, incluindo as propriedades de \n");
    printf("             cada membro (nome, UID, GID, permissoes, tamanho, data de modificacao)\n");
    printf("             e sua ordem no arquivo.\n\n");
    printf(" -h          Imprime uma pequena mensagem de ajuda com as opcoes disponiveis.\n");
}

// fecha o arquivo vina, desaloca a lista de arquivos e aborta 
// o programa com VINA_ERROR
void vinaAborta(FILE *vina_file, lista_avl_t *lista, int VINA_ERROR_CODE) {
    
    if (vina_file) fclose(vina_file);
    if (lista) listaAVLDestroi(lista);
    exit(VINA_ERROR_CODE);
}

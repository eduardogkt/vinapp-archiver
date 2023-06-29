#include "libUtilsVina++.h"
#include "libAVL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// verifica se o arquivo vina tem a extensao correta .vpp
int extensaoVina(char *vina_path) {
    
    char aux[NAME_SIZE];
    char *ext;
    
    strcpy(aux, vina_path);
    ext = strtok(aux, ".");
    ext = strtok(NULL, ".");
    if (ext == NULL) 
        return FALSE;
    return (strcmp(ext, "vpp") == 0);
}

// retorna verdadeiro caso o arquivo exista
int arquivoExiste(char *filepath) {

    struct stat fstatus;
    return (stat(filepath, &fstatus) == 0);
}

// padroniza o filename para que sejam arquivados como caminha relativo
char *caminhoRelativo(char *filepath) {

    if (filepath == NULL) return NULL;

    char *filename = (char *) malloc(sizeof(char) * NAME_SIZE);
    if (!filename) return NULL;
    memset(filename, 0, NAME_SIZE);

    if (filepath[0] == '.' && filepath[1] == '.') {
        strncat(filename, ".", NAME_SIZE-1);
        strncat(filename, "/", NAME_SIZE-1);
    }

    if (filepath[0] != '.') {
        strncat(filename, ".", NAME_SIZE-1);
        if (filepath[0] != '/') { 
            strncat(filename, "/", NAME_SIZE-1);
        }
    }
    strncat(filename, filepath, NAME_SIZE-1);
    return filename;
}

// funcao auxilizar que imprime os metadados de um arquivo
void imprimeCabecalho(vina_cabecalho_t *cabecalho) {

    printf("nome: %s\n",  cabecalho->nome);
    printf("path: %s\n",  cabecalho->path);
    printf("uid: %d\n", cabecalho->uid);
    printf("gid: %d\n", cabecalho->gid);
    printf("perms: %d\n", cabecalho->perms);
    printf("ordem: %d\n", cabecalho->ordem);
    printf("tamanho: %ld\n", cabecalho->tamanho);
    printf("data_mod: %ld\n", cabecalho->tmp_mod);
    printf("arq_ini: %ld\n", cabecalho->ini_arq);
    printf("\n");
}

// imprime todos os cabecalhos dos arquivos na arvore em ordem
void imprimeCabecalhos(lista_avl_t *lista) {

    no_t *no = lista->ini;
    while (no != NULL) {
        imprimeCabecalho(no->cabecalho);
        no = no->prox;
    }
    printf("\n");
}

// preenche o cabecalho com os metadados do arquivo
int preencheCabecalho(vina_cabecalho_t *cabecalho, char *filepath, ulong_t ini_arq, uint_t ordem) {

    struct stat fstatus;
    if (stat(filepath, &fstatus) < 0) {
        fprintf(stderr, "Erro: falha ao obter dados do arquivo \"%s\".\n", filepath);
        return FALSE;
    }
    // padroniza o nome do arquivo
    char *filename = caminhoRelativo(filepath);
    if (!filename) {
        printf("Erro: alocacao de memoria falhou.\n");
        return FALSE;
    }
    
    memset(cabecalho, 0, sizeof(vina_cabecalho_t));
    memcpy(cabecalho->nome, filepath, sizeof(cabecalho->nome));
    memcpy(cabecalho->path, filename, sizeof(cabecalho->path));
    cabecalho->uid     = fstatus.st_uid;
    cabecalho->gid     = fstatus.st_gid;
    cabecalho->perms   = fstatus.st_mode;
    cabecalho->ordem   = ordem;
    cabecalho->tamanho = fstatus.st_size;
    cabecalho->tmp_mod = fstatus.st_mtime;
    cabecalho->ini_arq = ini_arq;

    free(filename);
    return TRUE;
}

// insere os cabecalhos do arquivo vina na arvore AVL
int coletaCabecalhos(FILE *vina_file, no_t **raiz, lista_avl_t *lista, uint_t *ordem, ulong_t ini_dir) {

    vina_cabecalho_t cabecalho;
    ulong_t lido;

    fseek(vina_file, ini_dir, SEEK_SET);
    lido = fread(&cabecalho, sizeof(vina_cabecalho_t), 1, vina_file);
    while (!feof(vina_file)) {
        if (lido != 1) {
            fprintf(stderr, "Erro: falha ao coletar cabecalho.\n");
            return FALSE;
        }
        *raiz = insereAVL(*raiz, lista, &cabecalho);
        lido = fread(&cabecalho, sizeof(vina_cabecalho_t), 1, vina_file);
        (*ordem)++;
    }
    return TRUE;
}

// padroniza o filepath para o caminho realitivo e 
// procura o cabecalho correspontente
no_t *achaCabecalho(no_t *raiz, char *filepath) {

    char *filename = caminhoRelativo(filepath);
    if (!filename) return NULL;

    no_t *no_encontrado = buscaNo(raiz, filename);
    free(filename);
    return no_encontrado;
}

// atualiza informacoes do no com as informaçoes do cabecalho
void atualizaCabecalho(vina_cabecalho_t cabecalho, no_t *no) {

    no->cabecalho->uid     = cabecalho.uid;
    no->cabecalho->gid     = cabecalho.gid;
    no->cabecalho->perms   = cabecalho.perms;
    no->cabecalho->tamanho = cabecalho.tamanho;
    no->cabecalho->tmp_mod = cabecalho.tmp_mod;
}

// realiza a transferencia do conteudo de um arquivo de nome filename
// para o arquivo vina
int transfereParaVina(FILE *vina_file, char *filename) {

    FILE *file;
    ulong_t lido, escrito;
    char buffer[BUFFER_SIZE];

    if (!(file = fopen(filename, "r"))) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo \"%s\"\n", filename);
        return FALSE;
    }

    lido = fread(buffer, sizeof(char), BUFFER_SIZE, file);
    while (!feof(file)) {
        escrito = fwrite(buffer, sizeof(char), lido, vina_file);
        if (lido != escrito) {
            fprintf(stderr, "Erro: falha ao transferir \"%s\" para o arquivo vina.\n", filename);
            return FALSE;
        }
        lido = fread(buffer, sizeof(char), BUFFER_SIZE, file);
    }
    escrito = fwrite(buffer, sizeof(char), lido, vina_file);
    if (lido != escrito) {
        fprintf(stderr, "Erro: falha ao transferir \"%s\" para o arquivo vina.\n", filename);
        return FALSE;
    }
    fclose(file);

    return TRUE;
}

// transfere para o arquivo de nome filename o conteudo do arquivo 
// vina de tamanho tam_arq
int transfereParaArquivo(FILE *vina_file, char *filename, ulong_t tam_arq) {

    FILE *file;
    ulong_t lido, escrito, transferido = 0;
    char buffer[BUFFER_SIZE];

    if (!(file = fopen(filename, "w+"))) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo \"%s\".\n", filename);
        return FALSE;
    }

    // transferindo conteudo do arquivo para outro
    while (transferido < tam_arq && (lido = fread(buffer, sizeof(char), BUFFER_SIZE, vina_file)) > 0) {
        if (transferido + lido > tam_arq) {
            lido = tam_arq - transferido;
        }
        escrito = fwrite(buffer, sizeof(char), lido, file);
        if (escrito != lido) {
            fprintf(stderr, "Erro: falha ao transferir dados para o arquivo \"%s\".\n", filename);
            return FALSE;
        }
        transferido += lido;
    }
    fclose(file);
    return TRUE;
}

// desloca o conteudo de um arquivo 'deslocamento' bytes para frente
// considera que o long deslocamento passado en positivo
int deslocaParaFrente(FILE *vina_file, no_t *no, long deslocamento) {

    // salvando a localizacao anterior e a nova
    ulong_t ini_antigo = no->cabecalho->ini_arq;
    ulong_t ini_novo = ini_antigo + deslocamento;

    // transferindo o conteudo do arquivo para um arquivo temporario
    fseek(vina_file, ini_antigo, SEEK_SET);
    if (!transfereParaArquivo(vina_file, VINA_TMP_FILE, no->cabecalho->tamanho))
        return FALSE;
    
    // passando o arquivo temporario para a nova posicao no arquivo vina
    fseek(vina_file, ini_novo, SEEK_SET);
    if (!transfereParaVina(vina_file, VINA_TMP_FILE))
        return FALSE;
    
    system(VINA_RM_TMP_FILE);
    return TRUE;
}

// desloca conteudo do arquivo 'deslocamento' bytes para tras
// considera que o long deslocamento passado eh negetivo
int deslocaParaTras(FILE *vina_file, no_t *no, long deslocamento) {

    char buffer[BUFFER_SIZE];
    ulong_t lido, escrito, transferido = 0;
    ulong_t tamanho = no->cabecalho->tamanho;
    // 'ponteiros' indicando local de leitura e escrita
    ulong_t rp = no->cabecalho->ini_arq;
    ulong_t wp = no->cabecalho->ini_arq + deslocamento;

    while (transferido < tamanho) {
        fseek(vina_file, rp, SEEK_SET);
        lido = fread(buffer, sizeof(char), BUFFER_SIZE, vina_file);
        
        if (transferido + lido > tamanho) {
            lido = tamanho - transferido;
        }
        
        fseek(vina_file, wp, SEEK_SET);
        escrito = fwrite(buffer, sizeof(char), lido, vina_file);
        if (escrito  != lido) {
            fprintf(stderr, "Erro: falha na transferencia de dados.\n");
            return FALSE;
        }
        rp += lido;
        wp += lido;
        transferido += lido;
    }
    return TRUE;
}

// atualiza a localizacao do conteudo de todos os arquivos subsequentes
// ao arquivo substituido
int atualizaArquivosSubsequentes(FILE *vina_file, lista_avl_t *lista, no_t *no_encontrado, long deslocamento) {

    // se a diferenca de tamanho for maior que 0
    // o arquivo substituto eh maior - deslocando arquivos para frente
    if (deslocamento > 0) {
        no_t *no = lista->fim;
        while (no != NULL && (no != no_encontrado)) {
            if (!deslocaParaFrente(vina_file, no, deslocamento)) {
                return FALSE;
            }
            no->cabecalho->ini_arq += deslocamento;
            no = no->ant;
        }
    }
    // se a diferenca de tamanho for menor que 0
    // o arquivo substituto eh menor - deslocando arquivos para tras
    else if (deslocamento < 0) {
        no_t *no = no_encontrado->prox;
        while (no != NULL) {
            if (!deslocaParaTras(vina_file, no, deslocamento)) {
                return FALSE;
            }
            no->cabecalho->ini_arq += deslocamento;
            no = no->prox;
        }
    }
    return TRUE;
}

// retorna VINA_ARQ_EXTERNO caso o arquivo a ser inserido for mais 
// recente e retorna VINA_ARQ_INTERNO caso o arquivo que ja esta 
// no archive for mais recente
int arquivoMaisRecente(vina_cabecalho_t *externo, vina_cabecalho_t *interno) {

    if (externo->tmp_mod > interno->tmp_mod) 
        return VINA_ARQ_EXTERNO;
    else 
        return VINA_ARQ_INTERNO;
}

// atualiza o inteiro que indica o inicio do diretorio
// e grava os cabecalhos no arquivo vina
int atualizaDiretorio(FILE *vina_file, lista_avl_t *lista) {

    // atualizando o inicio do diretorio
    ulong_t ini_dir = sizeof(ulong_t) + lista->soma_tam_arqs;
    fseek(vina_file, 0, SEEK_SET);
    fwrite(&ini_dir, sizeof(ulong_t), 1, vina_file);

    // gravando os cabecalhos
    fseek(vina_file, ini_dir, SEEK_SET);
    no_t *no = lista->ini;
    while (no != NULL) {
        ulong_t escrito = fwrite(no->cabecalho, sizeof(vina_cabecalho_t), 1, vina_file);
        if (escrito != 1) {
            fprintf(stderr, "Erro: falha na escrita dos cabecalhos.\n");
            return FALSE;
        }
        no = no->prox;
    }
    return TRUE;
}

// calcula o novo tamanho do arquivo vina e trunca-o
void truncaArquivoVina(char *vina_path, lista_avl_t *lista) {

    ulong_t tam_dir = sizeof(vina_cabecalho_t) * lista->tamanho;
    ulong_t novo_tam = sizeof(ulong_t) + lista->soma_tam_arqs + tam_dir;
    truncate(vina_path, novo_tam);
}

// ajusta os ponteiros do no target e do no a ser movido
void atualizaPonteirosNos(lista_avl_t *lista, no_t *no_target, no_t *no_movido) {

    // "removendo" no a ser movido da lista
    if (no_movido == lista->ini) {
        lista->ini = no_movido->prox;
    }
    if (no_movido == lista->fim) {
        lista->fim = no_movido->ant;
    }
    if (no_movido->ant != NULL) {
        no_movido->ant->prox = no_movido->prox;
    }
    if (no_movido->prox != NULL) {
        no_movido->prox->ant = no_movido->ant;
    }

    // atualiza os ponteiros do no a ser movido
    no_movido->ant = no_target;
    no_movido->prox = no_target->prox;

    // ajusta os ponteiros dos nos adjacentes ao no a ser movido
    if (no_target->prox != NULL) {
        no_target->prox->ant = no_movido;
    }
    no_target->prox = no_movido;
    
    // atualiza o ponteiro de fim da lista caso o target seja o ultimo
    if (lista->fim == no_target)
        lista->fim = no_movido;

    // atualizando as informacoces dos nos apos o target
    no_t *no = no_target->prox;
    while (no != NULL) {
        no->cabecalho->ini_arq = no->ant->cabecalho->ini_arq + no->ant->cabecalho->tamanho;
        no->cabecalho->ordem = no->ant->cabecalho->ordem + 1;
        no = no->prox;
    }
}

// verificar casos nos quais nao eh preciso fazer movimentacoes
int atendeRequisitosMover(no_t *no_movido, no_t *no_target) {

    if (no_movido == NULL) {
        fprintf(stderr, "Erro: arquivo a ser movido nao encontrado.\n");
        return FALSE;
    }
    if (no_target == NULL) {
        fprintf(stderr, "Erro: arquivo target nao encontrado.\n");
        return FALSE;
    }
    if (no_movido->ant && no_movido->ant == no_target) {
        fprintf(stderr, "Erro: arquivo a ser movido ja esta imediatamente apos o target.\n");
        return FALSE;
    }
    if (no_movido == no_target) {
        fprintf(stderr, "Erro: Target e arquivo a ser movido sao iguais.\n");
        return FALSE;
    }
    return TRUE;
}

// insere o arquivo a ser movido imediatamente depois do target
int insereArquivoMovido(FILE *vina_file, lista_avl_t *lista, no_t *no_target, no_t *no_movido) {

    ulong_t tam_arq_movido = no_movido->cabecalho->tamanho;
    // abrir espaco no arquivo vina para o arquivo movido
    // indo do ultimo arquivo ate o target
    no_t *no = lista->fim;
    while (no != NULL && (no != no_target)) {
        if (!deslocaParaFrente(vina_file, no, tam_arq_movido)) {
            return FALSE;
        }
        no = no->ant;
    }

    // copiando conteudo do arquivo movido para depois do target
    ulong_t ini_target = no_target->cabecalho->ini_arq;
    ulong_t tam_target = no_target->cabecalho->tamanho;
    ulong_t ini_novo_movido = ini_target + tam_target;
    
    fseek(vina_file, ini_novo_movido, SEEK_SET);
    if (!transfereParaVina(vina_file, VINA_TMP_MOVE_FILE))
        return FALSE;
    
    system(VINA_RM_TMP_MOVE_FILE);
    return TRUE;
}

// remove o arquivo a ser movido da área de dados dos arquivos
int removeArquivoMovido(FILE *vina_file, lista_avl_t *lista, no_t *no_target, no_t *no_movido) {

    ulong_t tam_arq_movido = no_movido->cabecalho->tamanho;
    // move o conteudo dos arquivo subsequente para sobreescrever o movido
    // e atualiza as infos dos arquivos subsequentes
    no_t *no = no_movido->prox;
    while (no != NULL) {
        if (!deslocaParaTras(vina_file, no, -tam_arq_movido)) {
            return FALSE;
        }
        no->cabecalho->ini_arq -= tam_arq_movido;
        no->cabecalho->ordem--;
        no = no->prox;
    }
    return TRUE;
}

// recria a hierarquia de diretorios no diretorio atual, caso nao exista
void criaDiretorios(char *filename) {

    char diretorios[NAME_SIZE];
    char dirs_path[NAME_SIZE];
    char *dir;
    
    strcpy(diretorios, filename);
    dirs_path[0] = '\0';

    dir = strtok(diretorios, "/");
    strcat(dirs_path, dir);
    dir = strtok(NULL, "/");
    while (dir != NULL) {
        strcat(dirs_path, "/");
        strcat(dirs_path, dir);

        // verifica se o proximo eh o ultimo 
        if ((dir = strtok(NULL, "/")) == NULL)
            break;
        
        mkdir(dirs_path, 0777);
    }
}

// extrai todos o arquivo indicado por filepath
// caso filepath seja NULL, extrai todos arquivos
void extraiArquivos(FILE *vina_file, ulong_t ini_dir, char *filepath) {

    vina_cabecalho_t cabecalho;
    ulong_t lido;
    int extraido = FALSE;
    
    fseek(vina_file, ini_dir, SEEK_SET);
    lido = fread(&cabecalho,  sizeof(vina_cabecalho_t), 1, vina_file);
    while (!feof(vina_file)) {

        if (lido != 1) {
            fprintf(stderr, "Erro: falha ao extrair \"%s\"", cabecalho.path);
            continue; // passa para o proximo arquivo
        }
        char *filename = caminhoRelativo(filepath);

        if (filename == NULL || strcmp(cabecalho.path, filename) == 0) {
            // cria a hierarquia de diretorios
            criaDiretorios(cabecalho.path);

            // salva local de leitura do cabecalho
            ulong_t save = ftell(vina_file);
            fseek(vina_file, cabecalho.ini_arq, SEEK_SET);
            transfereParaArquivo(vina_file, cabecalho.path, cabecalho.tamanho);
            fseek(vina_file, save, SEEK_SET);
            extraido = TRUE;

            // caso o filename tenha sido especificado
            if (filename != NULL) {
                free(filename);
                return;
            }
            
        }
        if (filename != NULL) free(filename);
        lido = fread(&cabecalho, sizeof(vina_cabecalho_t), 1, vina_file);
    }
    if (!extraido) 
        fprintf(stderr, "Erro \"%s\" nao encontrado.\n", filepath);
}

// imprime os matadados do arquivo indicado pelo cabecalho
void imprimeAtributos(vina_cabecalho_t cabecalho) {

    // nome do usuario e nome do grupo
    struct passwd *usr = getpwuid(cabecalho.uid);
    struct group *grp = getgrgid(cabecalho.gid);
    
    // tempo de modificacao formatado
    struct tm *tm = localtime(&cabecalho.tmp_mod);
    char tmp_mod[17];
    strftime(tmp_mod, sizeof(tmp_mod), "%Y-%m-%d %H:%M", tm);

    // permissoes
    char permissoes[11] = "----------\0";
    permissoes[0] = (S_ISDIR(cabecalho.perms))  ? 'd' : '-';
    permissoes[1] = (cabecalho.perms & S_IRUSR) ? 'r' : '-';
    permissoes[2] = (cabecalho.perms & S_IWUSR) ? 'w' : '-';
    permissoes[3] = (cabecalho.perms & S_IXUSR) ? 'x' : '-';
    permissoes[4] = (cabecalho.perms & S_IRGRP) ? 'r' : '-';
    permissoes[5] = (cabecalho.perms & S_IWGRP) ? 'w' : '-';
    permissoes[6] = (cabecalho.perms & S_IXGRP) ? 'x' : '-';
    permissoes[7] = (cabecalho.perms & S_IROTH) ? 'r' : '-';
    permissoes[8] = (cabecalho.perms & S_IWOTH) ? 'w' : '-';
    permissoes[9] = (cabecalho.perms & S_IXOTH) ? 'x' : '-';

    printf("%s %s/%s %10ld %s %s\n",
        permissoes,         // permissoes
        usr->pw_name,       // nome do usuario
        grp->gr_name,       // nome do grupo
        cabecalho.tamanho,  // tamanho
        tmp_mod,            // data de modificacao
        cabecalho.path      // nome (path relativo)
    );
}

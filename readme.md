# Projeto 4 Programação II (CI1002)
Eduardo Gabriel Kenzo Tanaka (GRR20211791)

## Arquivador vina++
  Implementação do programa vina++, um arquivador básico (archiver), 
ou seja, um programa que salva em sequência uma coleção de arquivos 
(denominados membros) dentro de outro arquivo (denominado archiver) 
cuja estrutura permite recupera os arquivos originais 
individualmente.

---

## Execução do Programa
  O pacote de software gera um executável chamado vina++, que é 
executado da seguinte forma:
```
  ./vina++ <opção> <archive> [membro1 membro2 ...]
```
  O archive precisa, obrigatoriamente, ser um arquivo de extesão
.vpp. Caso seja passado um arquivo de extensão diferente, o
programa é abortado com uma mensagem de erro.

### Opções de Entrada
  As opções de entrada que podem ser especificadas são:
```
  -i : insere/acrescenta um ou mais membros ao archive. Caso 
  o membro já exista no archive, ele é substituído. Novos 
  membros são inseridos ao final do archive, respeitando a 
  ordem da linha de comando.

  -a : mesmo comportamento da opção -i, mas a substituição de 
  um membro existente ocorre apenas caso o parâmetro seja 
  mais recente que o arquivado.

  -m target : move o membro indicado na linha de comando para 
  imediatamente depois do membro target existente em archive. 

  -x : extrai os membros indicados de archive. Se os membros 
  não forem indicados, todos são ser extraídos.

  -r : remove os membros indicados de archive.

  -c : lista o conteúdo de archive em ordem, incluindo as 
  propriedades de cada membro (nome, UID, permissões, tamanho 
  e data de modificação) e sua ordem no arquivo.
    
  -h : imprime uma pequena mensagem de ajuda com as opções 
  disponíveis e encerra.
```
---

## Implementação

### Diretórios e Arquivos
  A estrutura do software entregue possui os diretórios:
```
  egkt21/
        /makefile : arquivo de makefile
        /readme : documentação do programa
        /src/ : arquivos fonte  
            /vina++.c : arquivo da função main()  
            /libVina++.c : implementação das funções de opções vina
            /libVina++.h : biblioteca das fuções de opções vina
            /libUtilsVina++.c : implementação das funções auxiliares
            /libUtilsVina++.h : biblioteca das funções auxiliares 
            /libAVL.c : implementação das funções de árvore AVL
            /libAVL.h : biblioteca das fuções de árvore AVL
```
### Arquivo Vina
  O arquivo vina foi organizado de forma que um inteiro no inicio do 
archiver indica a localização da área de diretório. Este inteiro é 
seguido pelo conteúdo dos arquivos, e então, o diretório com os 
cabeçalhos de todos arquivos presentes no archive.

  `ini_dir | conteúdo_arquivos | diretório`

### Estrutura do Código

#### Structs  
  O programa possui três estruturas principais:
```
  struct vina_cabecalho_t : cabecalho de metadados dos arquivos
  struct lista_t : lista para guadar ordem de inseção dos arquivos
  struct no_t : nó com ponterios e infos para a árvore e para lista
```
#### Árvore e Lista  
  Para evitar a leitura e escrita dos cabeçallhos a cada 
manipulação  do arquivo vina, os cabecalhos foram inseridos em um 
árvore AVL. Esta árvore também pode comportar-se como uma lista 
duplamente encadeada que conserva a ordem de inserção, uma vez que 
os nós possuem ponteiros de anterior e próximo.  
  As inserções remoções e buscas são feitas utilizando a estrura de 
árvore e a atualização dos dados dos cabeçalhos é feita utilizando 
a estrutura de lista.  

#### Transferência de Dados  
  Inicialmete, as funções de insere, remove e move foram 
implementadas utilizando um arquivo temporário para transferência de 
dados. Esta abordagem foi alterada de forma que a transferência seja 
feita diretamente do arquivo vina no caso em que o deslocamento de 
dados é negativo, ou seja, quando o conteúdo dos aquivos precisam 
ser movidos para trás. Esta otimização foi feita a fim de diminuir 
a quantidade de reads e writes. A função de deslocamento para frente 
continua utilizando a abordagem com arquivo temporário.  

#### Main  
  A função main lê a entrada, coleta o path para o arquivo vina, o 
path para a arquivo target e a opção. Em caso de falha, aborta o 
programa com mensagem descrevendo o erro. Senão, chama a função de 
acordo com a opção.

### Erros e Bugs
  No teste foi utilizado os diretórios: 
```
test/
    /aaa : arquivo de texto
    /bbb : arquivo de texto
    /ccc : arquivo de texto
    /ddd : arquivo de texto

tests/
     /cd.jpg    : imagem
     /cinco.txt : arquivo de texto
     
```
#### Erro de balanceamento
  O erro encontrado foi ao relaizar as seguintes operações:
```
  ./vina++ -i vina.vpp makefile tests/* test/* 
  ./vina++ -r vina.vpp tests/*
```
  O erro rodando com valgrind:
```
  Invalid read of size 8
    at 0x109CCF: rotEsquerda (libAVL.c:208)
    by 0x109FE7: casoEsqDir (libAVL.c:287)
    by 0x10A18F: balanceia (libAVL.c:322)
    by 0x10A348: removeAVL (libAVL.c:362)
    by 0x10BD8A: vinaRemove (libVina++.c:514)
    by 0x10D029: main (vina++.c:21)
  Address 0x10 is not stack'd, malloc'd or (recently) free'd
  Falha de segmentação (imagem do núcleo gravada)
```

  Uma mensagem foi inserida no local do erro e o programa é abortado
para evitar a falha de segmentação.

**Erro foi corrigido mudando a função de balanceamento para 
utilizar o fator de balanceamento para balancear ao invés de 
comparar as chaves dos nós com strcmp.**

  Implementação antiga:
```    
    int fb = fatorBalanceamento(no);

    if (fb > 1 && no->pai == NULL) // no eh raiz
        no = rotDireita(no);
    else 
    if (fb < -1 && no->pai == NULL) // no eh raiz
        no = rotEsquerda(no);
    else 
    if (fb > 1 && strcmp(nome, no->esq->cabecalho->path) < 0) 
        casoEsqEsq(no, nome); 
    else 
    if (fb < -1 && strcmp(nome, no->dir->cabecalho->path) > 0)
        casoDirDir(no, nome);
    else 
    if (fb > 1 && strcmp(nome, no->esq->cabecalho->path) >= 0)
        casoEsqDir(no, nome);
    else 
    if (fb < -1 && strcmp(nome, no->dir->cabecalho->path) < 0)
        casoDirEsq(no, nome);
    
    return no;
```

  Nova implementação:
```
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
```
CC = gcc
C_FLAGS = -Wall -std=c99 -g
EXECUTAVEL = vina++
SOURCE_DIR = src
OBJECT_DIR = obj

SOURCE_FILES = $(wildcard $(SOURCE_DIR)/*.c)
HEADER_FILES = $(wildcard $(SOURCE_DIR)/*.h)
OBJECT_FILES = $(patsubst $(SOURCE_DIR)/%,$(OBJECT_DIR)/%,$(SOURCE_FILES:.c=.o))

# regra padrao
all: makeobjdir $(EXECUTAVEL)

# regras de ligacao
$(EXECUTAVEL): $(OBJECT_FILES)
	$(CC) -o $@ $^

# regras de compilacao (.o colocados em um diretorio separado)
$(OBJECT_DIR)/$(EXECUTAVEL).o: $(SOURCE_DIR)/$(EXECUTAVEL).c $(HEADER_FILES)
	$(CC) $(C_FLAGS) -o $@ -c $<

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c $(SOURCE_DIR)/%.h
	$(CC) $(C_FLAGS) -o $@ -c $<

# utilidades
makeobjdir:
	@ mkdir -p $(OBJECT_DIR)

run: makeobjdir $(EXECUTAVEL)
	@ ./$(EXECUTAVEL)

clean:
	rm -f $(OBJECT_FILES)
	rmdir $(OBJECT_DIR)

purge: clean
	rm -f $(EXECUTAVEL)

rm_vpp:
	rm -f *.vpp

# $@ o alvo da regra atual
# $< a dependencia do alvo da regra atual
# $^ todas as dependencias do alvo da regra atual
# @  suprime (nao mostra na tela) o comando
# regra makedir caso main.o venha antes dos libs.o
# Makefile para Projeto Chat Concorrente - Etapa 1
# Compilador e flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O2
LDFLAGS = -pthread

# Diretórios
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Alvos principais
all: dirs bin/test_libtslog

# Criar diretórios
dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Executável de teste
bin/test_libtslog: build/test_libtslog.o build/libtslog.a
	@echo "Linkando executável de teste..."
	$(CXX) $(CXXFLAGS) build/test_libtslog.o -L$(BUILD_DIR) -ltslog $(LDFLAGS) -o $@

# Biblioteca estática libtslog
build/libtslog.a: build/libtslog.o
	@echo "Criando biblioteca libtslog.a..."
	ar rcs $@ $^

# Arquivos objeto
build/test_libtslog.o: src/test_libtslog.cpp include/libtslog.h
	@echo "Compilando teste..."
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c src/test_libtslog.cpp -o $@

build/libtslog.o: src/libtslog.cpp include/libtslog.h
	@echo "Compilando libtslog..."
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c src/libtslog.cpp -o $@

# Alvos .PHONY para comandos que não geram arquivos
.PHONY: all clean test

# Executar teste
test: all
	@echo "=== EXECUTANDO TESTE DA LIBTSLOG ==="
	./bin/test_libtslog

# Limpar build
clean:
	@echo "Limpando arquivos de build..."
	rm -rf $(BUILD_DIR) $(BIN_DIR) test_libtslog.log app.log



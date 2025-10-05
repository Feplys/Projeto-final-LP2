# Makefile para Projeto Chat Concorrente - ETAPA 3
# Compilador e flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O2
LDFLAGS = -pthread

# Diretórios
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin
SCRIPTS_DIR = scripts

# --- ARQUIVOS FONTE ---
# Biblioteca
LIBTSLOG_SOURCES = $(SRC_DIR)/libtslog.cpp
ERROR_HANDLER_SOURCES = $(SRC_DIR)/error_handler.cpp
# Chat
CHAT_COMMON_SOURCES = $(SRC_DIR)/chat_common.cpp
USER_DB_SOURCES = $(SRC_DIR)/user_database.cpp
CONNECTED_CLIENT_SOURCES = $(SRC_DIR)/connected_client.cpp
SERVER_SOURCES = $(SRC_DIR)/simple_chat_server.cpp
CLIENT_SOURCES = $(SRC_DIR)/simple_chat_client.cpp
# Mains
SERVER_MAIN_SOURCES = $(SRC_DIR)/chat_server_main.cpp
CLIENT_MAIN_SOURCES = $(SRC_DIR)/chat_client_main.cpp

# --- ARQUIVOS OBJETO ---
CHAT_OBJS = \
    $(BUILD_DIR)/libtslog.o \
    $(BUILD_DIR)/error_handler.o \
    $(BUILD_DIR)/chat_common.o \
    $(BUILD_DIR)/user_database.o \
    $(BUILD_DIR)/connected_client.o \
    $(BUILD_DIR)/simple_chat_client.o \
    $(BUILD_DIR)/simple_chat_server.o

SERVER_MAIN_OBJ = $(BUILD_DIR)/chat_server_main.o
CLIENT_MAIN_OBJ = $(BUILD_DIR)/chat_client_main.o

# --- EXECUTÁVEIS ---
CHAT_SERVER_BIN = $(BIN_DIR)/chat_server
CHAT_CLIENT_BIN = $(BIN_DIR)/chat_client
TEST_LIBTSLOG_BIN = $(BIN_DIR)/test_libtslog
TEST_LIBTSLOG_OBJ = $(BUILD_DIR)/test_libtslog.o

# Alvos principais
.PHONY: all clean dirs test-etapa1 test-etapa2 demo-server demo-client test-stress demo-visual help

all: dirs $(CHAT_SERVER_BIN) $(CHAT_CLIENT_BIN)

dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

# --- REGRAS DE COMPILAÇÃO ---

# Servidor de Chat
$(CHAT_SERVER_BIN): $(SERVER_MAIN_OBJ) $(CHAT_OBJS)
	@echo "🔗 Linkando servidor de chat..."
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

# Cliente de Chat
$(CHAT_CLIENT_BIN): $(CLIENT_MAIN_OBJ) $(CHAT_OBJS)
	@echo "🔗 Linkando cliente de chat..."
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

# Arquivos Main
$(SERVER_MAIN_OBJ): $(SERVER_MAIN_SOURCES)
	@echo "📝 Compilando $(notdir $<)..."
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(CLIENT_MAIN_OBJ): $(CLIENT_MAIN_SOURCES)
	@echo "📝 Compilando $(notdir $<)..."
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Arquivos Objeto Comuns
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "📝 Compilando $(notdir $<)..."
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Teste da Etapa 1
$(TEST_LIBTSLOG_BIN): $(TEST_LIBTSLOG_OBJ) $(BUILD_DIR)/libtslog.o
	@echo "🔗 Linkando teste da libtslog..."
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

$(TEST_LIBTSLOG_OBJ): $(SRC_DIR)/test_libtslog.cpp
	@echo "📝 Compilando $(notdir $<)..."
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@


# --- ALVOS DE TESTE E DEMONSTRAÇÃO ---

test-etapa1: $(TEST_LIBTSLOG_BIN)
	@echo "🧪 Executando teste da Etapa 1..."
	./$(TEST_LIBTSLOG_BIN)

test-etapa2: all
	@echo "🧪 EXECUTANDO TESTE DA ETAPA 2 (Cliente/Servidor)"
	@chmod +x $(SCRIPTS_DIR)/quick_test.sh
	@$(SCRIPTS_DIR)/quick_test.sh

test-stress: all
	@echo "🧪 EXECUTANDO TESTE DE ESTRESSE (Múltiplos Clientes)"
	@chmod +x $(SCRIPTS_DIR)/test_multiple_clients.sh
	@$(SCRIPTS_DIR)/test_multiple_clients.sh

demo-server: $(CHAT_SERVER_BIN)
	@echo "🖥️  EXECUTANDO SERVIDOR (Pressione Ctrl+C para parar)"
	./$(CHAT_SERVER_BIN)

demo-client: $(CHAT_CLIENT_BIN)
	@echo "💬 EXECUTANDO CLIENTE"
	./$(CHAT_CLIENT_BIN)

demo-visual: all
	@echo "🎬 EXECUTANDO DEMONSTRAÇÃO VISUAL"
	@chmod +x $(SCRIPTS_DIR)/visual_demo.sh
	@$(SCRIPTS_DIR)/visual_demo.sh

# --- LIMPEZA ---
clean:
	@echo "🧹 Limpando arquivos de build..."
	rm -rf $(BUILD_DIR) $(BIN_DIR) *.log *.db

help:
	@echo "Makefile do Projeto de Chat"
	@echo "--------------------------"
	@echo "Comandos:"
	@echo "  make all          - Compila o servidor e o cliente."
	@echo "  make clean        - Remove todos os arquivos compilados e logs."
	@echo "  make demo-server  - Executa o servidor de chat."
	@echo "  make demo-client  - Executa o cliente de chat."
	@echo "  make test-etapa1  - Roda o teste da biblioteca de log."
	@echo "  make test-etapa2  - Roda o teste rápido de cliente/servidor."
	@echo "  make test-stress  - Roda o teste de estresse com múltiplos clientes."
	@echo "  make demo-visual  - Roda a demonstração visual com múltiplos terminais."



# Makefile para Projeto Chat Concorrente - ETAPAS 1 E 2
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
LIBTSLOG_SOURCES = $(SRC_DIR)/libtslog.cpp
ERROR_HANDLER_SOURCES = $(SRC_DIR)/error_handler.cpp
CHAT_COMMON_SOURCES = $(SRC_DIR)/chat_common.cpp
CONNECTED_CLIENT_SOURCES = $(SRC_DIR)/connected_client.cpp
SERVER_SOURCES = $(SRC_DIR)/simple_chat_server.cpp
CLIENT_SOURCES = $(SRC_DIR)/simple_chat_client.cpp
SERVER_MAIN_SOURCES = $(SRC_DIR)/chat_server_main.cpp
CLIENT_MAIN_SOURCES = $(SRC_DIR)/chat_client_main.cpp
TEST_LIBTSLOG_SOURCES = $(SRC_DIR)/test_libtslog.cpp

# --- ARQUIVOS OBJETO ---
LIB_OBJS = \
    $(BUILD_DIR)/libtslog.o \
    $(BUILD_DIR)/error_handler.o \
    $(BUILD_DIR)/chat_common.o \
    $(BUILD_DIR)/connected_client.o \
    $(BUILD_DIR)/simple_chat_client.o \
    $(BUILD_DIR)/simple_chat_server.o

SERVER_MAIN_OBJ = $(BUILD_DIR)/chat_server_main.o
CLIENT_MAIN_OBJ = $(BUILD_DIR)/chat_client_main.o
TEST_LIBTSLOG_OBJ = $(BUILD_DIR)/test_libtslog.o

# --- EXECUTÁVEIS ---
CHAT_SERVER_BIN = $(BIN_DIR)/chat_server
CHAT_CLIENT_BIN = $(BIN_DIR)/chat_client
TEST_LIBTSLOG_BIN = $(BIN_DIR)/test_libtslog

# Alvos principais
.PHONY: all clean dirs etapa1 etapa2 test-etapa1 test-etapa2 test-stress demo-server demo-client demo-visual help

all: dirs $(CHAT_SERVER_BIN) $(CHAT_CLIENT_BIN) $(TEST_LIBTSLOG_BIN)

dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

# --- REGRAS DE COMPILAÇÃO ---

# Servidor de Chat
$(CHAT_SERVER_BIN): $(SERVER_MAIN_OBJ) $(LIB_OBJS)
	@echo "🔗 Linkando servidor de chat..."
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

# Cliente de Chat
$(CHAT_CLIENT_BIN): $(CLIENT_MAIN_OBJ) $(LIB_OBJS)
	@echo "🔗 Linkando cliente de chat..."
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

# Teste da Etapa 1
$(TEST_LIBTSLOG_BIN): $(TEST_LIBTSLOG_OBJ) $(filter-out $(BUILD_DIR)/simple_%, $(LIB_OBJS))
	@echo "🔗 Linkando teste da Etapa 1..."
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

# Regra genérica para compilar todos os arquivos .cpp da pasta src
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "📝 Compilando $(notdir $<)..."
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@


# --- ALVOS DE ETAPA ---

etapa1: test-etapa1
	@echo "✅ Etapa 1 compilada e testada!"

etapa2: all test-etapa2
	@echo "✅ Etapa 2 compilada e testada!"

# --- ALVOS DE TESTE E DEMONSTRAÇÃO ---

test-etapa1: $(TEST_LIBTSLOG_BIN)
	@echo "🧪 EXECUTANDO TESTE DA ETAPA 1 (libtslog)"
	./$(TEST_LIBTSLOG_BIN)

test-etapa2: $(CHAT_SERVER_BIN) $(CHAT_CLIENT_BIN)
	@echo "🧪 EXECUTANDO TESTE DA ETAPA 2 (quick_test.sh)"
	@chmod +x $(SCRIPTS_DIR)/quick_test.sh
	@$(SCRIPTS_DIR)/quick_test.sh

test-stress: $(CHAT_SERVER_BIN) $(CHAT_CLIENT_BIN)
	@echo "🧪 EXECUTANDO TESTE DE STRESS (test_multiple_clients.sh)"
	@chmod +x $(SCRIPTS_DIR)/test_multiple_clients.sh
	@$(SCRIPTS_DIR)/test_multiple_clients.sh

demo-server: $(CHAT_SERVER_BIN)
	@echo "🖥️  EXECUTANDO SERVIDOR (Pressione Ctrl+C para parar)"
	./$(CHAT_SERVER_BIN)

demo-client: $(CHAT_CLIENT_BIN)
	@echo "💬 EXECUTANDO CLIENTE (Digite /quit para sair)"
	./$(CHAT_CLIENT_BIN)

demo-visual: all
	@echo "🚀 INICIANDO DEMONSTRAÇÃO VISUAL..."
	@chmod +x $(SCRIPTS_DIR)/visual_demo.sh
	@$(SCRIPTS_DIR)/visual_demo.sh

# --- LIMPEZA E AJUDA ---
clean:
	@echo "🧹 Limpando arquivos de build..."
	rm -rf $(BUILD_DIR) $(BIN_DIR) *.log

help:
	@echo "Makefile do Projeto Chat Concorrente"
	@echo ""
	@echo "Uso: make [alvo]"
	@echo ""
	@echo "Alvos Principais:"
	@echo "  all           - Compila tudo (Etapa 1 e 2)"
	@echo "  etapa1        - Compila e testa apenas a Etapa 1"
	@echo "  etapa2        - Compila e testa a Etapa 2"
	@echo ""
	@echo "Testes:"
	@echo "  test-etapa1   - Roda o teste da biblioteca de log"
	@echo "  test-etapa2   - Roda o teste rápido da Etapa 2"
	@echo "  test-stress   - Roda o teste de estresse com múltiplos clientes"
	@echo ""
	@echo "Demonstração:"
	@echo "  demo-server   - Inicia o servidor em modo interativo"
	@echo "  demo-client   - Inicia o cliente em modo interativo"
	@echo "  demo-visual   - Roda uma demonstração com múltiplos terminais"
	@echo ""
	@echo "Manutenção:"
	@echo "  clean         - Remove todos os arquivos gerados"
	@echo "  help          - Mostra esta mensagem de ajuda"



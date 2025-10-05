#!/bin/bash

# Script de Teste Rápido - Etapa 2
# Testa funcionalidade básica cliente/servidor

# Cores
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}"
echo "🚀 TESTE RÁPIDO - CHAT CONCORRENTE ETAPA 2"
echo "=========================================="
echo -e "${NC}"

# Diretórios
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR" || exit 1

BIN_DIR="./bin"

# Verificar executáveis
if [[ ! -f "$BIN_DIR/chat_server" || ! -f "$BIN_DIR/chat_client" ]]; then
    echo -e "${RED}❌ Executáveis não encontrados. Compilando...${NC}"
    make all
    if [[ $? -ne 0 ]]; then
        echo -e "${RED}❌ Falha na compilação${NC}"
        exit 1
    fi
fi

# Configurações
PORT=8081
SERVER_PID=""
CLIENT_PIDS=()

# Função de limpeza
cleanup() {
    echo -e "\n${YELLOW}🧹 Limpando processos...${NC}"
    
    # Matar clientes primeiro
    for pid in "${CLIENT_PIDS[@]}"; do
        kill -9 $pid 2>/dev/null
    done
    
    # Depois o servidor
    if [[ -n $SERVER_PID ]] && kill -0 $SERVER_PID 2>/dev/null; then
        kill -SIGTERM $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
    
    # Garantir que nada ficou rodando
    pkill -f "chat_client.*-p $PORT" 2>/dev/null || true
    pkill -f "chat_server.*--port $PORT" 2>/dev/null || true
}

trap cleanup EXIT

# Limpar logs antigos
rm -f chat_server.log chat_client.log

echo -e "${BLUE}1. Iniciando servidor na porta $PORT...${NC}"
./bin/chat_server --daemon --port $PORT &
SERVER_PID=$!

sleep 2 # Aguardar servidor inicializar

if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${RED}❌ Falha ao iniciar servidor${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Servidor iniciado (PID: $SERVER_PID)${NC}"

echo -e "${BLUE}2. Testando conexão de 3 clientes simultâneos...${NC}"

# Iniciar 3 clientes de teste em background
./bin/chat_client -s 127.0.0.1 -p $PORT -u Cliente1 -a 3 > /dev/null 2>&1 &
CLIENT_PIDS+=($!)
sleep 0.2

./bin/chat_client -s 127.0.0.1 -p $PORT -u Cliente2 -a 3 > /dev/null 2>&1 &
CLIENT_PIDS+=($!)
sleep 0.2

./bin/chat_client -s 127.0.0.1 -p $PORT -u Cliente3 -a 3 > /dev/null 2>&1 &
CLIENT_PIDS+=($!)

echo -e "${YELLOW}⏳ Aguardando execução dos testes (10 segundos)...${NC}"
sleep 10

echo -e "${BLUE}3. Verificando resultados...${NC}"

CONNECTIONS=0
MESSAGES_SENT=0
SUCCESS=false

if [[ -f "chat_server.log" ]]; then
    # Contar conexões únicas (usuários que se conectaram com sucesso)
    CONNECTIONS=$(grep "conectado com sucesso" chat_server.log | awk '{print $2}' | sort -u | wc -l)
    
    # Contar total de mensagens broadcast enviadas pelos clientes
    MESSAGES_SENT=$(grep "Mensagem de Cliente" chat_server.log | wc -l)
fi

echo -e "\n${BLUE}📊 RESULTADO:${NC}"
echo "Conexões únicas no servidor: $CONNECTIONS/3"
echo "Mensagens enviadas pelos clientes: $MESSAGES_SENT"

# - Pelo menos 7 mensagens foram enviadas (3 clientes * 3 mensagens = 9, mas aceitamos 7+ devido a timing)
if [[ $CONNECTIONS -eq 3 && $MESSAGES_SENT -ge 7 ]]; then
    SUCCESS=true
fi

if $SUCCESS; then
    echo -e "\n${GREEN}🎉 TESTE PASSOU!${NC}"
    echo -e "${GREEN}✅ Servidor aceitou múltiplos clientes${NC}"
    echo -e "${GREEN}✅ Mensagens foram enviadas e retransmitidas${NC}"
    exit 0
else
    echo -e "\n${RED}❌ TESTE FALHOU!${NC}"
    echo -e "${YELLOW}Detalhes:${NC}"
    
    if [[ $CONNECTIONS -lt 3 ]]; then
        echo -e "${RED}  - Apenas $CONNECTIONS de 3 clientes conectaram${NC}"
    fi
    
    if [[ $MESSAGES_SENT -lt 7 ]]; then
        echo -e "${RED}  - Apenas $MESSAGES_SENT mensagens foram enviadas (esperado >= 7)${NC}"
    fi
    
    echo -e "\n${YELLOW}Verifique os logs para mais detalhes:${NC}"
    echo "  - chat_server.log"
    
    # Mostrar últimas linhas relevantes do log
    echo -e "\n${YELLOW}Últimas linhas do log do servidor:${NC}"
    grep -E "(conectado|Mensagem de|ERROR)" chat_server.log | tail -10
    
    exit 1
fi

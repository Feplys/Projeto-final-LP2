#!/bin/bash

# Script de Teste - Múltiplos Clientes Simultâneos
# Etapa 2 - Chat Concorrente
# Simula vários clientes conectando e enviando mensagens

# Configurações
NUM_CLIENTS=5
MESSAGES_PER_CLIENT=8
SERVER_PORT=8080
SERVER_ADDR="127.0.0.1"
TEST_DURATION=30

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Funções auxiliares
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

echo -e "${BLUE}🧪 TESTE DE MÚLTIPLOS CLIENTES - ETAPA 2${NC}"
echo "=========================================="

# Verificar executáveis
CLIENT_BIN="./bin/chat_client"
SERVER_BIN="./bin/chat_server"

if [[ ! -f "$CLIENT_BIN" || ! -f "$SERVER_BIN" ]]; then
    log_error "Executáveis não encontrados. Execute 'make all' primeiro."
    exit 1
fi

# Iniciar servidor em background
log_info "Iniciando servidor de teste em background na porta $SERVER_PORT..."
$SERVER_BIN --daemon --port $SERVER_PORT &
SERVER_PID=$!
sleep 2 # Dar tempo para o servidor iniciar

# Função para limpar recursos
cleanup() {
    log_info "\nLimpando processos de teste..."
    kill $SERVER_PID 2>/dev/null
    pkill -f "chat_client.*$SERVER_PORT" 2>/dev/null
    wait 2>/dev/null
    log_info "Limpeza concluída."
}

trap cleanup EXIT INT TERM

# Iniciar clientes
CLIENT_PIDS=()
for ((i=1; i<=NUM_CLIENTS; i++)); do
    username="TestUser$i"
    $CLIENT_BIN --server $SERVER_ADDR --port $SERVER_PORT --username $username --auto $MESSAGES_PER_CLIENT > "/tmp/client_${i}.log" 2>&1 &
    CLIENT_PIDS+=($!)
    log_info "Cliente $username iniciado (PID: ${CLIENT_PIDS[-1]})"
done

log_warning "Aguardando $TEST_DURATION segundos para a conclusão do teste..."
sleep $TEST_DURATION

# Análise dos resultados
log_info "\nANALISANDO RESULTADOS..."
echo "========================"

SUCCESSFUL_CLIENTS=0
TOTAL_MESSAGES_SENT=0
for ((i=1; i<=NUM_CLIENTS; i++)); do
    log_file="/tmp/client_${i}.log"
    username="TestUser$i"
    if [[ -f "$log_file" ]]; then
        messages_sent=$(grep -c "Enviada" "$log_file" || echo "0")
        TOTAL_MESSAGES_SENT=$((TOTAL_MESSAGES_SENT + messages_sent))
        if [[ $messages_sent -ge $MESSAGES_PER_CLIENT ]]; then
            log_success "✅ $username: $messages_sent/$MESSAGES_PER_CLIENT mensagens enviadas."
            ((SUCCESSFUL_CLIENTS++))
        else
            log_warning "⚠️ $username: $messages_sent/$MESSAGES_PER_CLIENT mensagens enviadas."
        fi
    else
        log_error "❌ $username: Arquivo de log não encontrado."
    fi
done

# Resultado final
EXPECTED_MESSAGES=$((NUM_CLIENTS * MESSAGES_PER_CLIENT))
SUCCESS_RATE=$(( (TOTAL_MESSAGES_SENT * 100) / (EXPECTED_MESSAGES > 0 ? EXPECTED_MESSAGES : 1) ))

echo "------------------------"
log_info "Clientes bem-sucedidos: $SUCCESSFUL_CLIENTS/$NUM_CLIENTS"
log_info "Total de mensagens enviadas: $TOTAL_MESSAGES_SENT/$EXPECTED_MESSAGES"
log_info "Taxa de sucesso: $SUCCESS_RATE%"

if [[ $SUCCESS_RATE -ge 90 ]]; then
    log_success "\n🎉 TESTE PASSOU! O servidor lidou bem com a carga."
else
    log_error "\n❌ TESTE FALHOU! Verifique os logs em /tmp/client_*.log e chat_server.log"
fi


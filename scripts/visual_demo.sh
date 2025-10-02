#!/bin/bash

# Script de Demonstração Visual - Etapa 2
# Abre múltiplos terminais para simular um chat em tempo real.

# --- CONFIGURAÇÕES ---
NUM_CLIENTS=5
SERVER_PORT=8080
TERMINAL_CMD="gnome-terminal --"
# ---------------------

# Cores
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}"
echo "🚀 DEMONSTRAÇÃO VISUAL - CHAT CONCORRENTE"
echo "=========================================="
echo -e "Este script vai abrir ${YELLOW}$((NUM_CLIENTS + 1))${CYAN} novas janelas de terminal."
echo -e "${NC}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR" || exit 1

echo -e "${BLUE}1. Compilando o projeto...${NC}"
make all
if [[ $? -ne 0 ]]; then
    echo -e "${RED}❌ Falha na compilação. Abortando.${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Projeto compilado!${NC}"

declare -a PIDS=()

cleanup() {
    echo -e "\n\n${YELLOW}🧹 Finalizando todos os processos da demonstração...${NC}"
    for pid in "${PIDS[@]}"; do
        kill -9 "$pid" 2>/dev/null
    done
    echo -e "${GREEN}✅ Limpeza concluída.${NC}"
}
trap cleanup EXIT

# Esta função será executada dentro de cada terminal de cliente.
# Ela gera mensagens aleatórias e as envia para o stdin do programa cliente.
run_chatter_client() {
    local PORT=$1
    local USERNAME=$2

    MESSAGES=("oi" "ola" "hello" "hi" "hola" "tudo bem?" "e aí?" "como vai?")
    
    (
      # O subshell ( ... ) permite que a saída seja canalizada via pipe |
      while true; do
        MSG=${MESSAGES[$RANDOM % ${#MESSAGES[@]}]}
        echo "$MSG"
        
        # Dorme por um tempo aleatório entre 2 e 5 segundos
        SLEEP_TIME=$(awk 'BEGIN{srand(); print 2+rand()*3}')
        sleep $SLEEP_TIME
      done
    ) | ./bin/chat_client --port "$PORT" --username "$USERNAME"
}

# Exporta a função para que ela fique disponível para os novos terminais
export -f run_chatter_client

echo -e "${BLUE}2. Iniciando o servidor em uma nova janela...${NC}"
$TERMINAL_CMD bash -c "./bin/chat_server --port $SERVER_PORT; read -p 'Pressione Enter para fechar...'" &
PIDS+=($!)
sleep 2

echo -e "${BLUE}3. Iniciando ${NUM_CLIENTS} clientes em modo 'chatter'...${NC}"
for (( i=1; i<=NUM_CLIENTS; i++ )); do
    USERNAME="Bot$i"
    # Chama a função exportada dentro do novo terminal
    $TERMINAL_CMD bash -c "run_chatter_client $SERVER_PORT $USERNAME; read -p 'Pressione Enter para fechar...'" &
    PIDS+=($!)
    sleep 0.5
done

echo -e "\n${GREEN}🎉 DEMONSTRAÇÃO INICIADA!${NC}"
echo -e "${YELLOW}Observe as janelas dos 'Bots' conversando entre si.${NC}"
echo -e "\n${CYAN}Quando terminar de observar, feche esta janela principal ou pressione Ctrl+C para encerrar todos os processos.${NC}"

wait



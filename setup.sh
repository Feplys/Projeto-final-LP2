#!/bin/bash

# Script de configuração e teste para o projeto Chat Concorrente - Etapa 1

set -e # Parar o script se algum comando falhar

# --- Cores para o terminal ---
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # Sem Cor

echo -e "${YELLOW}=== INICIANDO CONFIGURAÇÃO DO PROJETO ===${NC}"

# 1. Verificar se o Makefile existe
if [ ! -f "Makefile" ]; then
    echo -e "${RED}ERRO: Arquivo 'Makefile' não encontrado no diretório atual.${NC}"
    exit 1
fi

# 2. Limpar compilações anteriores
echo "--> Limpando builds antigos..."
make clean

# 3. Compilar o projeto
echo "--> Compilando a biblioteca e o programa de teste..."
make all
echo -e "${GREEN}Compilação concluída com sucesso!${NC}"

# 4. Executar o teste
echo "--> Executando o teste de concorrência..."
echo -e "${YELLOW}O programa a seguir vai pausar. Pressione ENTER para continuar.${NC}"
make test

# 5. Verificar o resultado
if [ -f "test_libtslog.log" ]; then
    echo -e "${GREEN}Teste finalizado. O arquivo 'test_libtslog.log' foi criado.${NC}"
else
    echo -e "${RED}ERRO: O arquivo de log 'test_libtslog.log' não foi gerado.${NC}"
fi

echo -e "${GREEN}=== CONFIGURAÇÃO E TESTE CONCLUÍDOS COM SUCESSO! ===${NC}"


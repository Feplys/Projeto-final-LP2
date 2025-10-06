# 💬 Servidor de Chat Multiusuário Concorrente

> Sistema de chat concorrente cliente-servidor desenvolvido em C++17 para a disciplina de Linguagem de Programação II (UFPB)

## 📋 Sobre o Projeto

Sistema de chat multiusuário desenvolvido em **C++17** com arquitetura cliente-servidor usando **sockets TCP**. O servidor gerencia múltiplas conexões simultâneas através de **threads POSIX**, garantindo concorrência segura com **mutexes**, **condition variables** e **monitores**.

### 🎯 Características Principais

- 🔗 **Servidor TCP concorrente** - Suporta até 50 clientes simultâneos
- 🧵 **Thread por conexão** - Cada cliente em thread dedicada
- 📢 **Mensagens broadcast** - Transmissão para todos os usuários
- 💬 **Mensagens privadas** - Comunicação one-to-one
- 🔐 **Sistema de autenticação** - Login e registro com senha
- 💾 **Persistência de dados** - Banco de usuários em arquivo
- 📝 **Logging thread-safe** - Biblioteca `libtslog` customizada
- 🧪 **Testes automatizados** - Suite completa de validação
- 🎨 **Filtro de conteúdo** - Censura de palavras ofensivas

---

## ✨ Status do Projeto

### Requisitos Obrigatórios ✅

- [✅] Servidor TCP concorrente aceitando múltiplos clientes
- [✅] Thread dedicada para cada cliente conectado
- [✅] Broadcast de mensagens para todos os usuários online
- [✅] Logging concorrente usando biblioteca `libtslog`
- [✅] Cliente CLI interativo para conexão e troca de mensagens
- [✅] Proteção de estruturas compartilhadas com mutexes

### Funcionalidades Opcionais ✅

- [✅] Sistema de autenticação (registro e login com senha)
- [✅] Mensagens privadas entre usuários
- [✅] Filtro de palavras ofensivas
- [✅] Persistência de usuários em arquivo
- [✅] Modo automático para testes

### Documentação ✅

- [✅] README.md completo
- [✅] Relatório técnico final (PDF)
- [✅] Diagramas de sequência
- [✅] Mapeamento requisitos → código
- [✅] Análise crítica com IA

---

## 🚀 Começando

### Pré-requisitos

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential g++ make

# Fedora/RHEL
sudo dnf install gcc-c++ make

# Arch Linux
sudo pacman -S base-devel
```

**Requisitos:**
- g++ 7.0+ com suporte a C++17
- GNU Make 4.0+
- Sistema Linux/Unix (testado em Ubuntu 22.04)

### Instalação

```bash
# 1. Clonar repositório
git clone https://github.com/Feplys/Projeto-final-LP2.git
cd Projeto-final-LP2

# 2. Compilar projeto
make clean && make all

# 3. Verificar binários
ls -lh bin/
# Deve mostrar: chat_server, chat_client, test_libtslog
```

---

## 📖 Como Usar

### Servidor

#### Modo Interativo (padrão)
```bash
./bin/chat_server
```

**Comandos disponíveis:**
```
servidor> help        # Mostra ajuda
servidor> stats       # Exibe estatísticas (conexões, mensagens)
servidor> clients     # Lista usuários online
servidor> stop        # Para o servidor gracefully
servidor> quit        # Alias para stop
servidor> cls         # Limpa a tela
```

#### Modo Daemon (para testes)
```bash
./bin/chat_server --daemon --port 8081
```

**Opções:**
- `--daemon` - Roda em modo background
- `--port N` ou `-p N` - Define porta (padrão: 8080)

---

### Cliente

#### Modo Interativo
```bash
./bin/chat_client
```

**Menu principal:**
```
1. Entrar (Login)       - Acessa com conta existente
2. Criar conta          - Registra novo usuário
3. Sair                 - Fecha o programa
```

**Comandos no chat:**
```
> /privado <usuário> <mensagem>   # Envia mensagem privada
> /quit                           # Sai do chat
> /help                           # Mostra ajuda
> /cls                            # Limpa tela
> <texto>                         # Mensagem pública (broadcast)
```

#### Modo Automático (testes)
```bash
./bin/chat_client --server 127.0.0.1 --port 8080 --username Bot1 --auto 10
```

**Parâmetros:**
- `--server ADDR` ou `-s ADDR` - Endereço do servidor
- `--port N` ou `-p N` - Porta do servidor
- `--username NAME` ou `-u NAME` - Nome de usuário
- `--auto N` ou `-a N` - Envia N mensagens automaticamente

---

## 🧪 Testes

### Teste Rápido (Etapa 2)
Valida funcionalidade básica com 3 clientes simultâneos:
```bash
make test-etapa2
```

**Verifica:**
- ✅ 3 conexões únicas
- ✅ Mínimo de 7 mensagens trocadas
- ✅ Servidor retransmite corretamente

---

### Teste de Stress
Simula 5 clientes enviando 8 mensagens cada:
```bash
make test-stress
```

**Critério de sucesso:** Taxa ≥ 90% (esperado: 100%)

---

### Teste da Biblioteca de Logging (Etapa 1)
Valida funcionamento thread-safe da `libtslog`:
```bash
make test-etapa1
```

**Verifica:**
- ✅ 10 threads × 20 mensagens (200 total)
- ✅ Sem mensagens corrompidas
- ✅ Timestamps em ordem

---

### Demonstração Visual
Abre múltiplas janelas de terminal com bots conversando:
```bash
make demo-visual
```

**⚠️ Requer:** `gnome-terminal` instalado

**O que faz:**
1. Abre janela com servidor
2. Abre 5 janelas com clientes bot
3. Bots conversam automaticamente entre si

---

### Compilação com Sanitizers

#### ThreadSanitizer (Race Conditions)
```bash
g++ -std=c++17 -fsanitize=thread -g -O1 \
    src/*.cpp -pthread -I include -o bin/chat_tsan

./bin/chat_tsan
```

#### AddressSanitizer (Memory Leaks)
```bash
g++ -std=c++17 -fsanitize=address -g -O1 \
    src/*.cpp -pthread -I include -o bin/chat_asan

./bin/chat_asan
```

---

## 📁 Estrutura do Projeto

```
chat-concorrente/
├── bin/                          # Executáveis compilados
│   ├── chat_server              # Servidor
│   ├── chat_client              # Cliente
│   └── test_libtslog            # Teste da lib de log
├── build/                        # Arquivos objeto (.o)
├── include/                      # Headers (.h)
│   ├── chat_common.h            # Constantes e estruturas
│   ├── chat_exceptions.h        # Exceções customizadas
│   ├── connected_client.h       # Cliente conectado
│   ├── error_handler.h          # Tratamento de erros
│   ├── libtslog.h               # Logger thread-safe
│   ├── simple_chat_client.h     # Classe do cliente
│   ├── simple_chat_server.h     # Classe do servidor
│   ├── thread_safe_queue.h      # Monitor (fila)
│   └── user_database.h          # BD de usuários
├── src/                          # Implementações (.cpp)
│   ├── chat_client_main.cpp     # Entry point cliente
│   ├── chat_common.cpp          # Utilitários
│   ├── chat_server_main.cpp     # Entry point servidor
│   ├── connected_client.cpp     # Gerenciamento de cliente
│   ├── error_handler.cpp        # Handlers de exceção
│   ├── libtslog.cpp             # Logger implementação
│   ├── simple_chat_client.cpp   # Lógica do cliente
│   ├── simple_chat_server.cpp   # Lógica do servidor
│   ├── test_libtslog.cpp        # Teste do logger
│   └── user_database.cpp        # Persistência
├── scripts/                      # Scripts de teste
│   ├── quick_test.sh            # Teste rápido (Etapa 2)
│   ├── test_multiple_clients.sh # Teste de stress
│   └── visual_demo.sh           # Demo visual
├── Makefile                      # Build system
├── README.md                     # Este arquivo
├── LPII-TRABALHO-FINAL.pdf       # Relatório técnico final
└── users.db                      # Banco de dados de usuários
```

---

## 🏗️ Arquitetura

### Componentes Principais

```
┌─────────────────────────────────────────────────────┐
│                  SimpleChatServer                    │
├─────────────────────────────────────────────────────┤
│ • Accept Thread        (aceita conexões)            │
│ • Client Threads       (1 por cliente)              │
│ • UserDatabase         (autenticação)               │
│ • Online Users Map     (clientes ativos)            │
└─────────────────────────────────────────────────────┘
                        │
                   TCP Socket
                        │
┌─────────────────────────────────────────────────────┐
│                  SimpleChatClient                    │
├─────────────────────────────────────────────────────┤
│ • Main Thread          (input do usuário)           │
│ • Receiver Thread      (recebe mensagens)           │
└─────────────────────────────────────────────────────┘
```

### Sincronização

| Mecanismo | Uso | Arquivo |
|-----------|-----|---------|
| `std::mutex` | Protege `online_users_` | `simple_chat_server.h` |
| `std::mutex` | Protege banco de dados | `user_database.h` |
| `std::condition_variable` | Fila produtor-consumidor | `thread_safe_queue.h` |
| `std::atomic<bool>` | Flags de controle | Vários |
| `std::shared_ptr` | Gerenciamento de clientes | `simple_chat_server.cpp` |

---

## 🔒 Segurança e Robustez

### Problemas Mitigados

✅ **Race Conditions:** Mutexes protegem todas as estruturas compartilhadas  
✅ **Deadlocks:** Locks sempre na mesma ordem, sem aninhamento  
✅ **Memory Leaks:** Smart pointers e RAII garantem limpeza  
✅ **Buffer Overflow:** Campos com tamanho fixo  

### Validação

Projeto testado com:
- **ThreadSanitizer:** 0 race conditions detectadas
- **Valgrind (memcheck):** 0 memory leaks
- **Valgrind (helgrind):** 0 deadlocks detectados

---

## 🎓 Conceitos Demonstrados

### Programação Concorrente
- ✅ Threads POSIX (`std::thread`)
- ✅ Exclusão mútua (`std::mutex`)
- ✅ Variáveis de condição (`std::condition_variable`)
- ✅ Monitores (`ThreadSafeQueue`)
- ✅ Operações atômicas (`std::atomic`)

### Rede
- ✅ Sockets TCP/IP
- ✅ Cliente-servidor
- ✅ Protocolo customizado
- ✅ Serialização de dados

### Engenharia de Software
- ✅ RAII (Resource Acquisition Is Initialization)
- ✅ Smart pointers (`shared_ptr`, `unique_ptr`)
- ✅ Padrão Singleton (Logger)
- ✅ Tratamento de exceções
- ✅ Testes automatizados

---



## 📝 Comandos Úteis

```bash
# Compilação e limpeza
make all          # Compila tudo
make clean        # Remove binários e objetos

# Executáveis
make demo-server  # Inicia servidor
make demo-client  # Inicia cliente

# Testes
make test-etapa1  # Teste da biblioteca de log
make test-etapa2  # Teste de comunicação básica
make test-stress  # Teste de carga
make demo-visual  # Demonstração visual

# Ajuda
make help         # Mostra todos os comandos
```

---

## 🐛 Troubleshooting

### Erro: "Address already in use"
```bash
# Porta 8080 ocupada
./bin/chat_server --port 8081
```

### Erro: "Connection refused"
```bash
# Verificar se servidor está rodando
ps aux | grep chat_server

# Verificar porta
netstat -tuln | grep 8080
```

### Testes falhando
```bash
# Recompilar do zero
make clean
make all

# Verificar logs
cat chat_server.log
cat chat_client.log
```




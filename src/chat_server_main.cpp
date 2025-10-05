#include "simple_chat_server.h"
#include "libtslog.h"
#include "error_handler.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>

using namespace chat;

std::unique_ptr<SimpleChatServer> server = nullptr;

void signal_handler(int signum) {
    std::cout << "\n\n🛑 Sinal recebido (" << signum << "). Parando servidor...\n" << std::endl;
    if (server) {
        server->stop();
    }
    exit(0);
}

void print_banner() {
    (void)system("clear");
    std::cout << R"(
🗨️  SERVIDOR DE CHAT CONCORRENTE
═══════════════════════════════════════════
)" << std::endl;
}

void print_help() {
    std::cout << "\n💡 COMANDOS DISPONÍVEIS:\n";
    std::cout << "  help     - Mostrar esta ajuda\n";
    std::cout << "  stats    - Exibir estatísticas do servidor\n";
    std::cout << "  clients  - Listar clientes online\n";
    std::cout << "  stop     - Parar o servidor\n";
    std::cout << "  quit     - Sair da aplicação\n";
    std::cout << "  cls      - Limpar tela\n";
    std::cout << std::endl;
}

void interactive_mode(SimpleChatServer& server_instance) {
    std::string command;
    while (server_instance.is_running()) {
        std::cout << "servidor> ";
        if (!std::getline(std::cin, command)) {
            break; 
        }
        
        if (command == "help") {
            print_help();
        } else if (command == "stats") {
            server_instance.print_stats();
        } else if (command == "clients") {
            auto usernames = server_instance.get_online_usernames(); // <<< CORRIGIDO AQUI
            std::cout << "\n👥 CLIENTES ONLINE (" << usernames.size() << "):\n";
            if (usernames.empty()) {
                std::cout << "  Nenhum cliente online.\n";
            } else {
                for (size_t i = 0; i < usernames.size(); ++i) {
                    std::cout << "  " << (i + 1) << ". " << usernames[i] << "\n";
                }
            }
            std::cout << std::endl;
        } else if (command == "stop" || command == "quit") {
            server_instance.stop();
            break;
        } else if (command == "cls" || command == "clear") {
            print_banner();
        } else if (!command.empty()) {
            std::cout << "❌ Comando desconhecido: " << command << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        tslog::Logger::getInstance().configure("chat_server.log", tslog::LogLevel::INFO, true, true);
    } catch (const std::exception& e) {
        std::cerr << "❌ Falha ao configurar logging: " << e.what() << std::endl;
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    int port = DEFAULT_PORT;
    // ... (processamento de argumentos pode ser adicionado aqui) ...

    print_banner();
    
    try {
        server = std::make_unique<SimpleChatServer>(port);
        std::cout << "🚀 Iniciando servidor na porta " << port << "...\n";
        if (!server->start()) {
            std::cerr << "❌ Falha fatal ao iniciar servidor." << std::endl;
            return 1;
        }
        std::cout << "✅ Servidor online. Clientes podem conectar.\n";
        interactive_mode(*server);
    } catch (const std::exception& e) {
        LOG_CRITICAL("Exceção fatal no servidor: " + std::string(e.what()));
        return 1;
    }

    std::cout << "👋 Servidor finalizado.\n";
    return 0;
}



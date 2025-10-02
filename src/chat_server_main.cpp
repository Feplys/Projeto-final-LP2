#include "simple_chat_server.h"
#include "libtslog.h"
#include "error_handler.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>

using namespace chat;

// Variável global para controle do servidor
std::unique_ptr<SimpleChatServer> server = nullptr;

// Handler para sinais (Ctrl+C)
void signal_handler(int signum) {
    std::cout << "\n\n🛑 Sinal recebido (" << signum << "). Parando servidor...\n" << std::endl;
    LOG_INFO("Sinal de interrupção recebido, parando servidor graciosamente");
    
    if (server) {
        server->stop();
    }
    
    exit(0);
}

void print_banner() {
    std::cout << R"(
🗨️  SERVIDOR DE CHAT CONCORRENTE - ETAPA 2
═══════════════════════════════════════════
)" << std::endl;
}

void interactive_mode(SimpleChatServer& server_instance) {
    std::cout << "\n🎮 MODO INTERATIVO ATIVO\n";
    std::cout << "Comandos: stats, clients, stop/quit, cls/clear\n\n";
    
    std::string command;
    while (server_instance.is_running()) {
        std::cout << "servidor> ";
        if (!std::getline(std::cin, command)) break;
        
        if (command == "stats") {
            server_instance.print_stats();
        } else if (command == "clients") {
            auto usernames = server_instance.get_connected_usernames();
            std::cout << "\n👥 CLIENTES CONECTADOS (" << usernames.size() << "):\n";
            if (usernames.empty()) {
                std::cout << "  Nenhum cliente conectado.\n";
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
            system("clear");
            print_banner();
        } else if (!command.empty()) {
            std::cout << "❌ Comando desconhecido.\n";
        }
    }
}

int main(int argc, char* argv[]) {
    print_banner();
    tslog::Logger::getInstance().configure("chat_server.log", tslog::LogLevel::INFO, true, true);
    
    int port = DEFAULT_PORT;
    bool interactive = true;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "--port" || arg == "-p") && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--daemon" || arg == "-d") {
            interactive = false;
        }
    }
    
    try {
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        
        server = std::make_unique<SimpleChatServer>(port);
        
        std::cout << "🚀 Iniciando servidor na porta " << port << "...\n";
        if (!server->start()) {
            std::cerr << "❌ Falha ao iniciar servidor" << std::endl;
            return 1;
        }
        std::cout << "✅ Servidor iniciado com sucesso!\n";
        
        if (interactive) {
            interactive_mode(*server);
        } else {
            std::cout << "🤖 Executando em modo daemon... (Pressione Ctrl+C para parar)\n";
            while (server->is_running()) {
                std::this_thread::sleep_for(std::chrono::seconds(60));
            }
        }
        
    } catch (const std::exception& e) {
        ErrorHandler::handle_exception(e);
        return 1;
    }
    
    std::cout << "👋 Servidor finalizado.\n";
    return 0;
}


#include "simple_chat_client.h"
#include "libtslog.h"
#include "error_handler.h"
#include <iostream>

using namespace chat;

void print_banner() {
    std::cout << R"(
💬 CLIENTE DE CHAT CONCORRENTE - ETAPA 2
════════════════════════════════════════
)" << std::endl;
}

void interactive_chat(SimpleChatClient& client) {
    std::cout << "\n🎮 CHAT INICIADO! Digite '/quit' para sair.\n\n";
    
    std::string input;
    while (client.is_connected()) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) {
            break; // EOF (Ctrl+D)
        }
        
        if (input == "/quit" || input == "/exit") {
            break;
        }
        
        if (!input.empty()) {
            if (!client.send_message(input)) {
                std::cout << "❌ Falha ao enviar mensagem. Desconectando.\n";
                break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    print_banner();
    tslog::Logger::getInstance().configure("chat_client.log", tslog::LogLevel::INFO, false, true);

    std::string server_addr = "127.0.0.1";
    int port = DEFAULT_PORT;
    std::string username;
    bool auto_mode = false;
    int num_test_messages = 10;

    // Processar argumentos
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "--server" || arg == "-s") && i + 1 < argc) server_addr = argv[++i];
        else if ((arg == "--port" || arg == "-p") && i + 1 < argc) port = std::stoi(argv[++i]);
        else if ((arg == "--username" || arg == "-u") && i + 1 < argc) username = argv[++i];
        else if (arg == "--auto" || arg == "-a") {
            auto_mode = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') num_test_messages = std::stoi(argv[++i]);
        }
    }

    if (username.empty()) {
        std::cout << "👤 Digite seu username: ";
        std::getline(std::cin, username);
        if (username.empty() || username.length() >= MAX_USERNAME_SIZE) {
            std::cerr << "❌ Username inválido." << std::endl;
            return 1;
        }
    }

    try {
        SimpleChatClient client(server_addr, port);
        
        std::cout << "🔗 Conectando em " << server_addr << ":" << port << " como '" << username << "'...\n";
        if (!client.connect_to_server(username)) {
            std::cerr << "❌ Falha ao conectar. Verifique se o servidor está rodando.\n";
            return 1;
        }
        std::cout << "✅ Conectado com sucesso!\n";

        if (auto_mode) {
            std::cout << "🤖 MODO TESTE AUTOMÁTICO - " << num_test_messages << " mensagens\n";
            client.set_auto_mode(true);
            for (int i = 1; i <= num_test_messages; ++i) {
                if (client.send_message("Mensagem de teste " + std::to_string(i))) {
                     std::cout << "📤 [" << i << "/" << num_test_messages << "] Enviada\n";
                } else break;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        } else {
            interactive_chat(client);
        }

        client.disconnect();

    } catch (const std::exception& e) {
        ErrorHandler::handle_exception(e);
        return 1;
    }
    
    std::cout << "👋 Cliente finalizado.\n";
    return 0;
}


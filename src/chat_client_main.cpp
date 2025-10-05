#include "simple_chat_client.h"
#include "libtslog.h"
#include <iostream>
#include <limits>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>
#include <random>

using namespace chat;

std::string get_password() {
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::string password;
    std::getline(std::cin, password);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;
    return password;
}

void print_banner() {
    (void)system("clear");
    std::cout << R"(
💬 CLIENTE DE CHAT CONCORRENTE
════════════════════════════════════════
)" << std::endl;
}

void print_chat_help() {
    std::cout << "\n💡 COMANDOS ESPECIAIS:\n";
    std::cout << "  /privado <nome> <mensagem> - Envia mensagem privada\n";
    std::cout << "  /quit                      - Sair do chat\n";
    std::cout << "  /cls                       - Limpar tela\n";
    std::cout << "\n📝 Para enviar mensagem pública, apenas digite e pressione ENTER\n" << std::endl;
}

void run_chat_session(SimpleChatClient& client) {
    print_banner();
    std::cout << "🎮 CHAT INICIADO! Bem-vindo, " << client.get_username() << "!\n";
    print_chat_help();

    std::string input;
    while (client.is_authenticated()) {
        std::cout << "> ";
        if (!std::getline(std::cin, input) || !client.is_authenticated()) {
            break;
        }

        if (input.empty()) continue;

        if (input.rfind("/privado ", 0) == 0) {
            std::stringstream ss(input);
            std::string command, target;
            ss >> command >> target;
            std::string message;
            std::getline(ss, message);
            if (!target.empty() && !message.empty()) {
                client.send_private(target, message.substr(1));
            } else {
                std::cout << "Uso: /privado <nome> <mensagem>\n";
            }
        } else if (input == "/quit") {
            break;
        } else if (input == "/cls") {
            print_banner();
        } else if (input == "/help") {
            print_chat_help();
        } else {
            client.send_broadcast(input);
        }
    }
}

void run_auto_mode(SimpleChatClient& client, int num_messages) {
    std::vector<std::string> messages = {
        "Olá pessoal!",
        "Como estão?",
        "Tudo bem por aí?",
        "Alguém online?",
        "Bom dia!",
        "Boa tarde!",
        "Tchau!",
        "Até logo!",
        "Legal!",
        "Bacana!"
    };
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> msg_dist(0, messages.size() - 1);
    std::uniform_int_distribution<> delay_dist(500, 2000); // 0.5 a 2 segundos
    
    for (int i = 0; i < num_messages && client.is_authenticated(); ++i) {
        std::string msg = messages[msg_dist(gen)];
        client.send_broadcast(msg);
        std::cout << "Enviada: " << msg << std::endl;
        
        if (i < num_messages - 1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(gen)));
        }
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main(int argc, char* argv[]) {
    try {
        tslog::Logger::getInstance().configure("chat_client.log", tslog::LogLevel::INFO, false, true);
    } catch (const std::exception& e) {
        std::cerr << "❌ Falha ao configurar logging: " << e.what() << std::endl;
        return 1;
    }

    std::string server_addr = "127.0.0.1";
    int port = DEFAULT_PORT;
    std::string username;
    int auto_messages = 0;
    bool auto_mode = false;

    // Parse argumentos
    for (int i = 1; i < argc; ++i) {
        if ((strcmp(argv[i], "--server") == 0 || strcmp(argv[i], "-s") == 0) && i + 1 < argc) {
            server_addr = argv[++i];
        } else if ((strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc) {
            port = std::atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--username") == 0 || strcmp(argv[i], "-u") == 0) && i + 1 < argc) {
            username = argv[++i];
        } else if ((strcmp(argv[i], "--auto") == 0 || strcmp(argv[i], "-a") == 0) && i + 1 < argc) {
            auto_messages = std::atoi(argv[++i]);
            auto_mode = true;
        }
    }

    // Modo automático (para testes)
    if (auto_mode && !username.empty()) {
        SimpleChatClient client(server_addr, port);
        
        // Tentar registrar primeiro, se falhar, fazer login
        std::string password = "senha123";
        bool connected = client.connect_and_register(username, password);
        
        if (!connected) {
            connected = client.connect_and_login(username, password);
        }
        
        if (connected) {
            run_auto_mode(client, auto_messages);
            client.disconnect();
            return 0;
        } else {
            std::cerr << "❌ Falha na autenticação automática\n";
            return 1;
        }
    }

    // Modo interativo
    while (true) {
        SimpleChatClient client(server_addr, port);

        print_banner();
        std::cout << "--- MENU PRINCIPAL ---\n";
        std::cout << "1. Entrar (Login)\n";
        std::cout << "2. Criar conta (Registar)\n";
        std::cout << "3. Sair\n";
        std::cout << "Escolha uma opção: ";

        int choice;
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = 0;
        } else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        std::string name, password;
        bool auth_success = false;

        switch (choice) {
            case 1: 
                std::cout << "\n--- LOGIN ---\n";
                std::cout << "Digite seu nome: ";
                std::getline(std::cin, name);
                std::cout << "Digite sua senha: ";
                password = get_password();
                auth_success = client.connect_and_login(name, password);
                break;
            case 2:
                std::cout << "\n--- CRIAR CONTA ---\n";
                std::cout << "Escolha um nome: ";
                std::getline(std::cin, name);
                std::cout << "Escolha uma senha: ";
                password = get_password();
                if (!Utils::is_valid_username(name) || !Utils::is_valid_password(password)) {
                    std::cerr << "❌ Nome ou senha inválido. Nome: 3-15 caracteres alfanuméricos. Senha: min 4 caracteres.\n";
                    auth_success = false;
                } else {
                    auth_success = client.connect_and_register(name, password);
                }
                break;
            case 3:
                std::cout << "👋 Até logo!\n";
                return 0;
            default:
                std::cout << "❌ Opção inválida. Tente novamente.\n";
                break;
        }

        if (auth_success) {
            run_chat_session(client);
        }
        
        client.disconnect();
        
        if (choice > 0 && choice < 3) {
            std::cout << "\nPressione Enter para voltar ao menu...";
            std::cin.get();
        }
    }

    return 0;
}

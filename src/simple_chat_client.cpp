#include "simple_chat_client.h"
#include "libtslog.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace chat {

// Função de leitura robusta, lê do socket até encontrar um '\n'
static std::string read_line_from_socket(int socket_fd) {
    std::string line;
    char buffer;
    ssize_t bytes_read;
    while ((bytes_read = recv(socket_fd, &buffer, 1, 0)) > 0) {
        if (buffer == '\n') {
            break;
        }
        line += buffer;
    }
    if (bytes_read <= 0) return ""; // Conexão fechada ou erro
    return line;
}

SimpleChatClient::SimpleChatClient(const std::string& server_addr, int port)
    : socket_fd_(-1), is_connected_(false), is_authenticated_(false),
      server_address_(server_addr), server_port_(port), should_stop_(false) {}

SimpleChatClient::~SimpleChatClient() {
    disconnect();
}

bool SimpleChatClient::establish_connection() {
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ == -1) { std::cerr << "❌ Falha ao criar socket.\n"; return false; }

    struct sockaddr_in server_addr_struct;
    memset(&server_addr_struct, 0, sizeof(server_addr_struct));
    server_addr_struct.sin_family = AF_INET;
    server_addr_struct.sin_port = htons(server_port_);
    inet_pton(AF_INET, server_address_.c_str(), &server_addr_struct.sin_addr);

    if (connect(socket_fd_, (struct sockaddr*)&server_addr_struct, sizeof(server_addr_struct)) == -1) {
        std::cerr << "❌ Falha ao conectar ao servidor.\n";
        cleanup_connection();
        return false;
    }
    return true;
}

bool SimpleChatClient::handle_auth_response(const std::string& response_data, const std::string& original_username) {
    if (response_data.empty()) {
        std::cerr << "❌ Servidor fechou a conexão inesperadamente.\n";
        return false;
    }
    Message response_msg = Message::deserialize(response_data);
    if (response_msg.type == MessageType::AUTH_SUCCESS) {
        is_connected_.store(true);
        is_authenticated_.store(true);
        username_ = original_username;
        should_stop_.store(false);
        receiver_thread_ = std::thread(&SimpleChatClient::receiver_thread_func, this);
        std::cout << "✅ " << response_msg.content << std::endl;
        return true;
    } else {
        std::cerr << "❌ " << response_msg.content << std::endl;
        cleanup_connection();
        return false;
    }
}

bool SimpleChatClient::connect_and_login(const std::string& username, const std::string& password) {
    if (!establish_connection()) return false;
    Message request_msg(MessageType::LOGIN_REQUEST, username, "");
    strncpy(request_msg.password, password.c_str(), MAX_PASSWORD_SIZE - 1);
    if (!send_data(request_msg.serialize())) return false;
    
    std::string response_data = read_line_from_socket(socket_fd_);
    return handle_auth_response(response_data, username);
}

bool SimpleChatClient::connect_and_register(const std::string& username, const std::string& password) {
    if (!establish_connection()) return false;
    Message request_msg(MessageType::REGISTER_REQUEST, username, "");
    strncpy(request_msg.password, password.c_str(), MAX_PASSWORD_SIZE - 1);
    if (!send_data(request_msg.serialize())) return false;

    std::string response_data = read_line_from_socket(socket_fd_);
    return handle_auth_response(response_data, username);
}

void SimpleChatClient::disconnect() {
    if (!is_connected_.exchange(false)) return;
    LOG_INFO("A desconectar do servidor...");
    should_stop_.store(true);
    if (is_authenticated_.load()) {
        Message disconnect_msg(MessageType::DISCONNECT_REQUEST, username_, "");
        send_data(disconnect_msg.serialize());
    }
    cleanup_connection();
    if (receiver_thread_.joinable()) receiver_thread_.join();
    LOG_INFO("Desconectado.");
}

void SimpleChatClient::send_broadcast(const std::string& message) {
    if (!is_authenticated_.load()) return;
    Message msg(MessageType::CHAT_BROADCAST, username_, message);
    send_data(msg.serialize());
}

void SimpleChatClient::send_private(const std::string& target, const std::string& message) {
    if (!is_authenticated_.load()) return;
    Message msg(MessageType::PRIVATE_MESSAGE, username_, message);
    strncpy(msg.target_user, target.c_str(), MAX_USERNAME_SIZE - 1);
    send_data(msg.serialize());
}

void SimpleChatClient::receiver_thread_func() {
    LOG_INFO("Thread de receção iniciada.");
    while (!should_stop_.load()) {
        std::string data = read_line_from_socket(socket_fd_);
        if (data.empty()) {
            if (!should_stop_.load()) {
                is_connected_.store(false);
                is_authenticated_.store(false);
                std::cout << "\n\n❌ Conexão com o servidor perdida. Pressione Enter para sair.\n" << std::endl;
            }
            break;
        }
        Message msg = Message::deserialize(data);
        process_chat_message(msg);
    }
    LOG_INFO("Thread de receção finalizada.");
}

void SimpleChatClient::process_chat_message(const Message& msg) {
    switch (msg.type) {
        case MessageType::CHAT_BROADCAST:
            std::cout << "\n[" << Utils::get_timestamp_str() << "] " << msg.username << ": " << msg.content << std::endl; break;
        case MessageType::PRIVATE_MESSAGE:
            std::cout << "\n[" << Utils::get_timestamp_str() << "] (privado de " << msg.username << "): " << msg.content << std::endl; break;
        case MessageType::SERVER_MESSAGE:
            std::cout << "\n>>> SERVIDOR: " << msg.content << std::endl; break;
        case MessageType::ERROR_MSG:
            std::cout << "\n!!! ERRO: " << msg.content << std::endl; break;
        default:
            LOG_WARNING("Mensagem de tipo desconhecido recebida: " + std::to_string(static_cast<int>(msg.type))); break;
    }
}

void SimpleChatClient::cleanup_connection() {
    if (socket_fd_ != -1) {
        shutdown(socket_fd_, SHUT_RDWR);
        close(socket_fd_);
        socket_fd_ = -1;
    }
}

bool SimpleChatClient::send_data(const std::string& data) {
    if (socket_fd_ == -1) return false;
    std::string full_data = data + "\n";
    return send(socket_fd_, full_data.c_str(), full_data.length(), MSG_NOSIGNAL) > 0;
}

} // namespace chat



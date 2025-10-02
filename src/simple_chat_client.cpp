#include "simple_chat_client.h"
#include "libtslog.h"
#include "error_handler.h"
#include <iostream>
#include <cstring>

namespace chat {

SimpleChatClient::SimpleChatClient(const std::string& server_addr, int port)
    : socket_fd_(-1), status_(ConnectionStatus::DISCONNECTED),
      server_address_(server_addr), server_port_(port),
      should_stop_(false), messages_sent_(0), messages_received_(0) {

    if (!NetworkUtils::is_valid_ip(server_addr) && server_addr != "localhost") {
        throw NetworkException("Endereço de servidor inválido: " + server_addr);
    }

    if (!NetworkUtils::is_valid_port(port)) {
        throw NetworkException("Porta inválida: " + std::to_string(port));
    }

    LOG_INFO("Cliente criado para " + server_address_ + ":" + std::to_string(server_port_));
}

SimpleChatClient::~SimpleChatClient() {
    disconnect();
    LOG_INFO("Cliente destruído");
}

bool SimpleChatClient::connect_to_server(const std::string& username) {
    std::lock_guard<std::mutex> lock(connection_mutex_);

    if (status_.load() == ConnectionStatus::CONNECTED) {
        LOG_WARNING("Cliente já está conectado como " + username_);
        return false;
    }

    if (username.empty() || username.length() >= MAX_USERNAME_SIZE) {
        LOG_ERROR("Username inválido: '" + username + "'");
        return false;
    }

    LOG_INFO("=== CONECTANDO AO SERVIDOR ===");
    LOG_INFO("Servidor: " + server_address_ + ":" + std::to_string(server_port_));
    LOG_INFO("Username: " + username);

    try {
        update_status(ConnectionStatus::CONNECTING);
        username_ = username;

        if (!setup_connection()) {
            update_status(ConnectionStatus::ERROR_STATE);
            return false;
        }

        Message connect_msg(MessageType::CONNECT, username_, "conectando");
        std::string connect_data = connect_msg.serialize() + "\n";

        if (!send_string_to_server(connect_data)) {
            LOG_ERROR("Falha ao enviar mensagem de conexão");
            cleanup_connection();
            update_status(ConnectionStatus::ERROR_STATE);
            return false;
        }

        LOG_INFO("Mensagem de conexão enviada");

        should_stop_.store(false);
        receiver_thread_ = std::thread(&SimpleChatClient::receiver_thread_func, this);

        update_status(ConnectionStatus::CONNECTED);
        LOG_INFO("Conectado com sucesso como " + username_);

        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Exceção ao conectar: " + std::string(e.what()));
        cleanup_connection();
        update_status(ConnectionStatus::ERROR_STATE);
        return false;
    }
}

void SimpleChatClient::disconnect() {
    if (status_.load() == ConnectionStatus::DISCONNECTED) {
        return;
    }

    LOG_INFO("=== DESCONECTANDO DO SERVIDOR ===");

    should_stop_.store(true);
    update_status(ConnectionStatus::DISCONNECTED);

    if (socket_fd_ != -1 && !username_.empty()) {
        try {
            Message disconnect_msg(MessageType::DISCONNECT, username_, "desconectando");
            std::string disconnect_data = disconnect_msg.serialize() + "\n";
            send_string_to_server(disconnect_data);
        } catch (...) {
            // Ignorar erros na desconexão
        }
    }

    cleanup_connection();

    if (receiver_thread_.joinable()) {
        receiver_thread_.join();
    }

    print_stats();
    LOG_INFO("Desconectado completamente");
}

bool SimpleChatClient::is_connected() const {
    return status_.load() == ConnectionStatus::CONNECTED;
}

bool SimpleChatClient::send_message(const std::string& message) {
    if (!is_connected()) {
        LOG_WARNING("Tentativa de envio sem estar conectado");
        return false;
    }

    if (message.empty() || message.length() >= MAX_MESSAGE_SIZE) {
        LOG_WARNING("Mensagem inválida (vazia ou muito longa)");
        return false;
    }

    try {
        Message chat_msg(MessageType::CHAT, username_, message);
        std::string msg_data = chat_msg.serialize() + "\n";

        if (send_string_to_server(msg_data)) {
            messages_sent_++;
            LOG_DEBUG("Mensagem enviada: " + message);
            return true;
        } else {
            LOG_ERROR("Falha ao enviar mensagem: " + message);
            return false;
        }

    } catch (const std::exception& e) {
        LOG_ERROR("Exceção ao enviar mensagem: " + std::string(e.what()));
        return false;
    }
}

bool SimpleChatClient::setup_connection() {
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ == -1) {
        LOG_ERROR("Falha ao criar socket: " + std::string(strerror(errno)));
        return false;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port_);

    if (server_address_ == "localhost") {
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    } else {
        server_addr.sin_addr.s_addr = inet_addr(server_address_.c_str());
    }

    if (server_addr.sin_addr.s_addr == INADDR_NONE) {
        LOG_ERROR("Endereço de servidor inválido: " + server_address_);
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    if (connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        LOG_ERROR("Falha ao conectar com " + server_address_ + ":" +
                  std::to_string(server_port_) + " - " + strerror(errno));
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    LOG_INFO("Socket conectado com sucesso (fd: " + std::to_string(socket_fd_) + ")");
    return true;
}

void SimpleChatClient::cleanup_connection() {
    if (socket_fd_ != -1) {
        close(socket_fd_);
        socket_fd_ = -1;
        LOG_DEBUG("Socket cliente fechado");
    }
}

void SimpleChatClient::receiver_thread_func() {
    LOG_INFO("Thread de recepção iniciada para " + username_);

    while (!should_stop_.load() && is_connected()) {
        try {
            std::string msg_data = receive_string_from_server();

            if (msg_data.empty()) {
                if (!should_stop_.load()) {
                    LOG_INFO("Servidor fechou a conexão");
                    update_status(ConnectionStatus::DISCONNECTED);
                }
                break;
            }

            Message msg = Message::deserialize(msg_data);
            if (msg.is_valid()) {
                process_received_message(msg);
                messages_received_++;
            } else {
                LOG_WARNING("Mensagem inválida recebida do servidor");
            }

        } catch (const std::exception& e) {
            if (!should_stop_.load()) {
                LOG_ERROR("Erro na thread de recepção: " + std::string(e.what()));
                update_status(ConnectionStatus::ERROR_STATE);
            }
            break;
        }
    }

    LOG_INFO("Thread de recepção finalizada para " + username_);
}

std::string SimpleChatClient::receive_string_from_server() {
    if (socket_fd_ == -1) {
        return "";
    }

    char buffer[BUFFER_SIZE];
    ssize_t received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);

    if (received <= 0) {
        return "";  // Conexão fechada ou erro
    }

    buffer[received] = '\0';

    std::string result(buffer);
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }

    return result;
}

bool SimpleChatClient::send_string_to_server(const std::string& data) {
    if (socket_fd_ == -1) {
        return false;
    }

    ssize_t sent = send(socket_fd_, data.c_str(), data.length(), MSG_NOSIGNAL);
    return sent > 0;
}

void SimpleChatClient::process_received_message(const Message& msg) {
    LOG_DEBUG("Mensagem recebida: " + msg.to_string());

    switch (msg.type) {
        case MessageType::SERVER_MSG:
            handle_server_message(msg);
            break;

        case MessageType::CHAT:
        case MessageType::CONNECT:
        case MessageType::DISCONNECT:
            display_message(msg);
            break;

        case MessageType::ERROR_MSG:
            LOG_ERROR("Erro do servidor: " + std::string(msg.content));
            if (!auto_mode_) {
                std::cout << "!!! ERRO DO SERVIDOR: " << msg.content << std::endl;
            }
            break;

        default:
            LOG_WARNING("Tipo de mensagem desconhecido: " + std::to_string(static_cast<int>(msg.type)));
            break;
    }
}

void SimpleChatClient::handle_server_message(const Message& msg) {
    LOG_INFO("Mensagem do servidor: " + std::string(msg.content));
    if (!auto_mode_) {
        std::cout << ">>> SERVIDOR: " << msg.content << std::endl;
    }
}

void SimpleChatClient::display_message(const Message& msg) {
    if (!auto_mode_) {
        std::cout << NetworkUtils::format_message_for_display(msg) << std::endl;
    }
    LOG_DEBUG("Mensagem exibida: " + NetworkUtils::format_message_for_display(msg));
}

void SimpleChatClient::update_status(ConnectionStatus new_status) {
    status_.store(new_status);

    std::string status_str;
    switch (new_status) {
        case ConnectionStatus::DISCONNECTED: status_str = "DESCONECTADO"; break;
        case ConnectionStatus::CONNECTING: status_str = "CONECTANDO"; break;
        case ConnectionStatus::CONNECTED: status_str = "CONECTADO"; break;
        case ConnectionStatus::ERROR_STATE: status_str = "ERRO"; break;
    }

    LOG_DEBUG("Status alterado para: " + status_str);
}

void SimpleChatClient::print_stats() const {
    std::cout << "\n=== ESTATÍSTICAS DO CLIENTE ===" << std::endl;
    std::cout << "Username: " << username_ << std::endl;
    std::cout << "Servidor: " << server_address_ << ":" << server_port_ << std::endl;
    std::cout << "Status: ";

    switch (status_.load()) {
        case ConnectionStatus::DISCONNECTED: std::cout << "DESCONECTADO"; break;
        case ConnectionStatus::CONNECTING: std::cout << "CONECTANDO"; break;
        case ConnectionStatus::CONNECTED: std::cout << "CONECTADO"; break;
        case ConnectionStatus::ERROR_STATE: std::cout << "ERRO"; break;
    }
    std::cout << std::endl;

    std::cout << "Mensagens enviadas: " << messages_sent_.load() << std::endl;
    std::cout << "Mensagens recebidas: " << messages_received_.load() << std::endl;
    std::cout << "===============================" << std::endl;

    LOG_INFO("Estatísticas do cliente exibidas");
}

} 



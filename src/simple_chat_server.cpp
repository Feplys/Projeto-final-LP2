#include "simple_chat_server.h"
#include "libtslog.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace chat {

SimpleChatServer::SimpleChatServer(int port)
    : server_socket_(-1), port_(port), running_(false) {}

SimpleChatServer::~SimpleChatServer() {
    if (is_running()) stop();
}

bool SimpleChatServer::start() {
    if (running_.exchange(true)) return false;
    if (!setup_server_socket()) { running_.store(false); return false; }
    accept_thread_ = std::thread(&SimpleChatServer::accept_connections, this);
    LOG_INFO("Servidor iniciado na porta " + std::to_string(port_));
    return true;
}

void SimpleChatServer::stop() {
    if (!running_.exchange(false)) return;
    LOG_INFO("A parar o servidor...");
    cleanup_server_socket();
    if (accept_thread_.joinable()) accept_thread_.join();
    {
        std::lock_guard<std::mutex> lock(online_users_mutex_);
        for (auto const& [_, client] : online_users_) {
            if(client) client->disconnect();
        }
        online_users_.clear();
    }
    LOG_INFO("Servidor parado.");
}

void SimpleChatServer::accept_connections() {
    LOG_INFO("Thread de aceitação iniciada");
    while (running_.load()) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running_.load()) LOG_ERROR("Falha no accept: " + std::string(strerror(errno)));
            continue;
        }
        
        char client_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_str, INET_ADDRSTRLEN);
        std::string client_ip(client_ip_str);
        LOG_INFO("Nova conexão de " + client_ip);
        std::thread(&SimpleChatServer::handle_client, this, client_socket, client_ip).detach();
    }
    LOG_INFO("Thread de aceitação finalizada.");
}

void SimpleChatServer::handle_client(int client_socket, std::string client_addr) {
    std::string read_buffer; // O "caderno de rascunho" local desta thread
    std::string username;
    try {
        std::string initial_data = Utils::read_line(client_socket, read_buffer);
        if (initial_data.empty()) {
            LOG_WARNING("Cliente " + client_addr + " desconectou antes do handshake.");
            close(client_socket);
            return;
        }

        Message auth_msg = Message::deserialize(initial_data);
        username = auth_msg.username;
        std::string password = auth_msg.password;
        bool success = false;
        std::string response_text;

        if (auth_msg.type == MessageType::LOGIN_REQUEST) {
            if (user_db_.validate_user(username, password)) {
                success = true; response_text = "Login bem-sucedido!";
            } else { response_text = "Nome ou senha inválidos."; }
        } else if (auth_msg.type == MessageType::REGISTER_REQUEST) {
            if (user_db_.add_user(username, password)) {
                success = true; response_text = "Conta criada com sucesso!";
            } else { response_text = "Nome de utilizador já existe."; }
        } else { response_text = "Pedido inválido."; }

        Message auth_response(success ? MessageType::AUTH_SUCCESS : MessageType::AUTH_FAILURE, "SERVER", response_text);
        std::string response_data = auth_response.serialize() + "\n";
        send(client_socket, response_data.c_str(), response_data.length(), MSG_NOSIGNAL);

        if (!success) {
            close(client_socket);
            return;
        }

        auto client_ptr = std::make_shared<ConnectedClient>(client_socket, username);
        add_online_user(username, client_ptr);
        
        while (running_.load() && client_ptr->is_active()) {
            std::string data = client_ptr->receive_data_blocking(read_buffer);
            if (data.empty()) break;
            Message msg = Message::deserialize(data);
            process_client_message(msg, client_ptr);
        }
    } catch (...) {}
    if (!username.empty()) remove_online_user(username);
}

void SimpleChatServer::process_client_message(const Message& msg, std::shared_ptr<ConnectedClient> client) {
    switch (msg.type) {
        case MessageType::CHAT_BROADCAST: broadcast_message(msg); break;
        case MessageType::PRIVATE_MESSAGE: send_private_message(msg); break;
        case MessageType::DISCONNECT_REQUEST: if(client) client->disconnect(); break;
        default: break;
    }
}

void SimpleChatServer::add_online_user(const std::string& username, std::shared_ptr<ConnectedClient> client) {
    std::lock_guard<std::mutex> lock(online_users_mutex_);
    online_users_[username] = client;
    client->start_sender_thread();
    Message join_notification(MessageType::SERVER_MESSAGE, "SERVER", "*** " + username + " entrou no chat ***");
    for (const auto& pair : online_users_) {
        if (pair.first != username) pair.second->queue_message(join_notification);
    }
}

void SimpleChatServer::remove_online_user(const std::string& username) {
    {
        std::lock_guard<std::mutex> lock(online_users_mutex_);
        if (online_users_.count(username)) {
            online_users_.erase(username);
        }
    }
    Message leave_notification(MessageType::SERVER_MESSAGE, "SERVER", "*** " + username + " saiu do chat ***");
    broadcast_message(leave_notification);
}

void SimpleChatServer::broadcast_message(const Message& msg) {
    std::lock_guard<std::mutex> lock(online_users_mutex_);
    for (const auto& pair : online_users_) pair.second->queue_message(msg);
}

void SimpleChatServer::send_private_message(const Message& msg) {
    std::string target = msg.target_user, sender = msg.username;
    std::shared_ptr<ConnectedClient> target_client, sender_client;
    {
        std::lock_guard<std::mutex> lock(online_users_mutex_);
        if (online_users_.count(target)) target_client = online_users_.at(target);
        if (online_users_.count(sender)) sender_client = online_users_.at(sender);
    }
    if (target_client && sender_client) {
        target_client->queue_message(msg);
        sender_client->queue_message(msg);
    } else if (sender_client) {
        Message error_msg(MessageType::ERROR_MSG, "SERVER", "Utilizador '" + target + "' não encontrado.");
        sender_client->queue_message(error_msg);
    }
}

bool SimpleChatServer::setup_server_socket() { /* ... código sem alterações ... */ return true; }
void SimpleChatServer::cleanup_server_socket() { /* ... código sem alterações ... */ }
int SimpleChatServer::get_online_user_count() const { /* ... código sem alterações ... */ }
std::vector<std::string> SimpleChatServer::get_online_usernames() const { /* ... código sem alterações ... */ }
void SimpleChatServer::print_stats() const { /* ... código sem alterações ... */ }

} // namespace chat



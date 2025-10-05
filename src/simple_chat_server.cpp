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
    : server_socket_(-1), port_(port), running_(false), 
      total_connections_(0), total_messages_processed_(0) {}

SimpleChatServer::~SimpleChatServer() {
    if (is_running()) stop();
}

bool SimpleChatServer::start() {
    if (running_.exchange(true)) return false;
    if (!setup_server_socket()) { 
        running_.store(false); 
        return false; 
    }
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

bool SimpleChatServer::setup_server_socket() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        LOG_ERROR("Falha ao criar socket: " + std::string(strerror(errno)));
        return false;
    }

    // Permite reutilizar a porta imediatamente após fechar
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG_WARNING("Falha ao definir SO_REUSEADDR");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOG_ERROR("Falha no bind da porta " + std::to_string(port_) + ": " + std::string(strerror(errno)));
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }

    if (listen(server_socket_, MAX_CLIENTS) < 0) {
        LOG_ERROR("Falha no listen: " + std::string(strerror(errno)));
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }

    LOG_INFO("Socket do servidor configurado com sucesso na porta " + std::to_string(port_));
    return true;
}

void SimpleChatServer::cleanup_server_socket() {
    if (server_socket_ != -1) {
        shutdown(server_socket_, SHUT_RDWR);
        close(server_socket_);
        server_socket_ = -1;
    }
}

void SimpleChatServer::accept_connections() {
    LOG_INFO("Thread de aceitação iniciada");
    while (running_.load()) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (running_.load()) {
                LOG_ERROR("Falha no accept: " + std::string(strerror(errno)));
            }
            continue;
        }
        
        total_connections_++;
        
        char client_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_str, INET_ADDRSTRLEN);
        std::string client_ip(client_ip_str);
        LOG_INFO("Nova conexão de " + client_ip);
        
        std::thread(&SimpleChatServer::handle_client, this, client_socket, client_ip).detach();
    }
    LOG_INFO("Thread de aceitação finalizada.");
}

void SimpleChatServer::handle_client(int client_socket, std::string client_addr) {
    std::string read_buffer;
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
                // Verificar se usuário já está online
                {
                    std::lock_guard<std::mutex> lock(online_users_mutex_);
                    if (online_users_.count(username)) {
                        success = false;
                        response_text = "Utilizador já está online.";
                    } else {
                        success = true;
                        response_text = "Login bem-sucedido!";
                    }
                }
            } else {
                response_text = "Nome ou senha inválidos.";
            }
        } else if (auth_msg.type == MessageType::REGISTER_REQUEST) {
            if (user_db_.add_user(username, password)) {
                success = true;
                response_text = "Conta criada com sucesso!";
            } else {
                response_text = "Nome de utilizador já existe.";
            }
        } else {
            response_text = "Pedido inválido.";
        }

        Message auth_response(success ? MessageType::AUTH_SUCCESS : MessageType::AUTH_FAILURE, 
                             "SERVER", response_text);
        std::string response_data = auth_response.serialize() + "\n";
        
        ssize_t sent = send(client_socket, response_data.c_str(), response_data.length(), MSG_NOSIGNAL);
        if (sent <= 0) {
            LOG_ERROR("Falha ao enviar resposta de autenticação para " + client_addr);
            close(client_socket);
            return;
        }

        if (!success) {
            close(client_socket);
            return;
        }

        LOG_INFO(username + " conectado com sucesso de " + client_addr);

        auto client_ptr = std::make_shared<ConnectedClient>(client_socket, username);
        add_online_user(username, client_ptr);
        
        while (running_.load() && client_ptr->is_active()) {
            std::string data = client_ptr->receive_data_blocking(read_buffer);
            if (data.empty()) break;
            
            Message msg = Message::deserialize(data);
            total_messages_processed_++;
            process_client_message(msg, client_ptr);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Exceção ao lidar com cliente " + client_addr + ": " + e.what());
    }
    
    if (!username.empty()) {
        LOG_INFO(username + " desconectado.");
        remove_online_user(username);
    }
}

void SimpleChatServer::process_client_message(const Message& msg, std::shared_ptr<ConnectedClient> client) {
    switch (msg.type) {
        case MessageType::CHAT_BROADCAST:
            broadcast_message(msg);
            LOG_INFO("Mensagem de " + std::string(msg.username) + " retransmitida para " + 
                    std::to_string(get_online_user_count() - 1) + " clientes");
            break;
        case MessageType::PRIVATE_MESSAGE:
            send_private_message(msg);
            LOG_INFO("Mensagem privada de " + std::string(msg.username) + 
                    " para " + std::string(msg.target_user));
            break;
        case MessageType::DISCONNECT_REQUEST:
            if(client) client->disconnect();
            break;
        default:
            LOG_WARNING("Tipo de mensagem desconhecido recebido: " + 
                       std::to_string(static_cast<int>(msg.type)));
            break;
    }
}

void SimpleChatServer::add_online_user(const std::string& username, 
                                       std::shared_ptr<ConnectedClient> client) {
    {
        std::lock_guard<std::mutex> lock(online_users_mutex_);
        online_users_[username] = client;
    }
    
    client->start_sender_thread();
    
    Message join_notification(MessageType::SERVER_MESSAGE, "SERVER", 
                             "*** " + username + " entrou no chat ***");
    
    std::lock_guard<std::mutex> lock(online_users_mutex_);
    for (const auto& pair : online_users_) {
        if (pair.first != username && pair.second) {
            pair.second->queue_message(join_notification);
        }
    }
}

void SimpleChatServer::remove_online_user(const std::string& username) {
    {
        std::lock_guard<std::mutex> lock(online_users_mutex_);
        if (online_users_.count(username)) {
            online_users_.erase(username);
        }
    }
    
    Message leave_notification(MessageType::SERVER_MESSAGE, "SERVER", 
                              "*** " + username + " saiu do chat ***");
    broadcast_message(leave_notification);
}

void SimpleChatServer::broadcast_message(const Message& msg) {
    std::lock_guard<std::mutex> lock(online_users_mutex_);
    for (const auto& pair : online_users_) {
        if (pair.second) {
            pair.second->queue_message(msg);
        }
    }
}

void SimpleChatServer::send_private_message(const Message& msg) {
    std::string target = msg.target_user;
    std::string sender = msg.username;
    
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
        Message error_msg(MessageType::ERROR_MSG, "SERVER", 
                         "Utilizador '" + target + "' não encontrado.");
        sender_client->queue_message(error_msg);
    }
}

int SimpleChatServer::get_online_user_count() const {
    std::lock_guard<std::mutex> lock(online_users_mutex_);
    return static_cast<int>(online_users_.size());
}

std::vector<std::string> SimpleChatServer::get_online_usernames() const {
    std::lock_guard<std::mutex> lock(online_users_mutex_);
    std::vector<std::string> usernames;
    usernames.reserve(online_users_.size());
    for (const auto& pair : online_users_) {
        usernames.push_back(pair.first);
    }
    return usernames;
}

void SimpleChatServer::print_stats() const {
    std::cout << "\n📊 ESTATÍSTICAS DO SERVIDOR\n";
    std::cout << "══════════════════════════════\n";
    std::cout << "  Clientes online: " << get_online_user_count() << "\n";
    std::cout << "  Total de conexões: " << total_connections_.load() << "\n";
    std::cout << "  Mensagens processadas: " << total_messages_processed_.load() << "\n";
    std::cout << "  Utilizadores registrados: " << user_db_.get_user_count() << "\n";
    std::cout << "══════════════════════════════\n" << std::endl;
}

} 

#include "simple_chat_server.h"
#include "libtslog.h"
#include <iostream>
#include <algorithm>
#include <cstring> 

namespace chat {


// Construtor e Destrutor do Servidor

SimpleChatServer::SimpleChatServer(int port)
    : server_socket_(-1), running_(false), port_(port),
      total_connections_(0), total_messages_(0) {
    LOG_INFO("Objeto Servidor criado para a porta " + std::to_string(port));
}

SimpleChatServer::~SimpleChatServer() {
    if (is_running()) {
        stop();
    }
    LOG_INFO("Objeto Servidor destruído");
}


// Controle do Servidor

bool SimpleChatServer::start() {
    if (running_.exchange(true)) {
        LOG_WARNING("Servidor já está em execução.");
        return false;
    }

    if (!setup_server_socket()) {
        running_.store(false);
        return false;
    }

    // Iniciar a thread que aceita conexões
    accept_thread_ = std::thread(&SimpleChatServer::accept_connections, this);
    
    LOG_INFO("Servidor iniciado com sucesso na porta " + std::to_string(port_));
    return true;
}

void SimpleChatServer::stop() {
    if (!running_.exchange(false)) {
        return; 
    }

    LOG_INFO("A parar o servidor...");

    // Fechar o socket do servidor para desbloquear o accept()
    cleanup_server_socket();

    // Aguardar a thread de aceitação finalizar
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }
    
    // Desconectar todos os clientes
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto const& [username, client] : clients_) {
            if(client) client->disconnect();
        }
        clients_.clear();
    }

    LOG_INFO("Servidor parado com sucesso.");
}

// Threads Principais: Aceitação e Tratamento de Cliente


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

        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        LOG_INFO("Nova conexão de " + client_ip + ", a aguardar handshake...");
        
        // A thread se torna independente (detached). O servidor não precisa mais gerenciá-la.
        // Ela se limpará automaticamente quando terminar.
        std::thread(&SimpleChatServer::handle_client, this, client_socket, client_ip).detach();
    }

    LOG_INFO("Thread de aceitação finalizada.");
}

void SimpleChatServer::handle_client(int client_socket, const std::string& client_addr) {
    total_connections_++;
    std::string username;
    std::shared_ptr<ConnectedClient> client_ptr = nullptr;

    try {
        std::string handshake_data = receive_string_from_socket(client_socket);
        if (handshake_data.empty()) {
            LOG_WARNING("Cliente " + client_addr + " desconectou antes do handshake.");
            close(client_socket);
            return;
        }

        Message connect_msg = Message::deserialize(handshake_data);
        if (connect_msg.type != MessageType::CONNECT || !connect_msg.is_valid()) {
            LOG_WARNING("Handshake inválido de " + client_addr);
            close(client_socket);
            return;
        }
        
        username = connect_msg.username;
        client_ptr = std::make_shared<ConnectedClient>(client_socket, username);

        if (!add_client(username, client_ptr)) {
            LOG_WARNING("Username '" + username + "' já em uso. Rejeitando conexão.");
            Message error_msg(MessageType::ERROR_MSG, "SERVER", "Username já em uso.");
            send_string_to_socket(client_socket, error_msg.serialize() + "\n");
            close(client_socket);
            return;
        }

        LOG_INFO("Cliente " + username + " ("+ client_addr +") conectado com sucesso.");
        client_ptr->start_sender_thread();
        

        Message welcome_msg(MessageType::SERVER_MSG, "SERVER", "Bem-vindo ao chat, " + username + "!");
        client_ptr->queue_message(welcome_msg);

        Message join_notification(MessageType::SERVER_MSG, "SERVER", "*** " + username + " entrou no chat ***");
        broadcast_message(join_notification, username); // Excluir o próprio usuário da notificação

        // Loop principal de recepção de mensagens
        while (running_.load() && client_ptr->is_active()) {
            std::string data = receive_string_from_socket(client_socket);
            if (data.empty()) {
                break; // Cliente desconectou
            }
            Message msg = Message::deserialize(data);
            if(msg.is_valid()) {
                process_client_message(msg, client_ptr);
            }
        }

    } catch (const std::exception& e) {
        LOG_ERROR("Exceção na thread do cliente " + username + ": " + e.what());
    }

    // Ações de limpeza ao desconectar
    if (!username.empty()) {
        LOG_INFO("Cliente " + username + " ("+ client_addr +") desconectado.");
        remove_client(username);

        // Notificar todos que o cliente saiu
        Message leave_notification(MessageType::SERVER_MSG, "SERVER", "*** " + username + " saiu do chat ***");
        broadcast_message(leave_notification);
    } else {
         close(client_socket); 
    }
}



// Processamento e Broadcast de Mensagens

void SimpleChatServer::process_client_message(const Message& msg, std::shared_ptr<ConnectedClient> client) {
    total_messages_++;
    LOG_INFO("Mensagem de '" + std::string(msg.username) + "': " + std::string(msg.content));
    
    switch (msg.type) {
        case MessageType::CHAT:
            // Ao retransmitir, excluir o remetente original
            broadcast_message(msg, client->get_username());
            break;
        case MessageType::DISCONNECT:
            if(client) client->disconnect();
            break;
        default:
            LOG_WARNING("Tipo de mensagem não esperado do cliente: " + std::to_string(static_cast<int>(msg.type)));
            break;
    }
}

void SimpleChatServer::remove_client(const std::string& username) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    if (clients_.count(username)) {
        clients_.erase(username);
        LOG_INFO("Cliente " + username + " removido da lista.");
    }
}

void SimpleChatServer::broadcast_message(const Message& msg, const std::string& exclude_user) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    int count = 0;
    for (auto const& [username, client] : clients_) {
        if (username != exclude_user && client && client->is_active()) {
            client->queue_message(msg);
            count++;
        }
    }
    // Logar retransmissão para mensagens de chat e do servidor
    if (msg.type == MessageType::CHAT || msg.type == MessageType::SERVER_MSG) {
        LOG_INFO("Mensagem '" + std::string(msg.content) + "' retransmitida para " + std::to_string(count) + " clientes.");
    }
}

bool SimpleChatServer::add_client(const std::string& username, std::shared_ptr<ConnectedClient> client) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    if (clients_.count(username)) {
        return false; // Username já existe
    }
    clients_[username] = client;
    return true;
}



// Utilitários de Rede e Configuração de Socket


std::string SimpleChatServer::receive_string_from_socket(int socket_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) {
        return ""; // Conexão fechada ou erro
    }

    buffer[bytes_received] = '\0';
    std::string result(buffer);
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}

bool SimpleChatServer::send_string_to_socket(int socket_fd, const std::string& data) {
    ssize_t sent = send(socket_fd, data.c_str(), data.length(), MSG_NOSIGNAL);
    return sent > 0;
}


bool SimpleChatServer::setup_server_socket() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        LOG_CRITICAL("Falha ao criar socket do servidor: " + std::string(strerror(errno)));
        return false;
    }

    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG_CRITICAL("Falha ao configurar SO_REUSEADDR: " + std::string(strerror(errno)));
        cleanup_server_socket();
        return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOG_CRITICAL("Falha ao fazer bind na porta " + std::to_string(port_) + ": " + std::string(strerror(errno)));
        cleanup_server_socket();
        return false;
    }

    if (listen(server_socket_, MAX_CLIENTS) < 0) {
        LOG_CRITICAL("Falha ao escutar na porta: " + std::string(strerror(errno)));
        cleanup_server_socket();
        return false;
    }
    
    return true;
}

void SimpleChatServer::cleanup_server_socket() {
    if (server_socket_ != -1) {
        shutdown(server_socket_, SHUT_RDWR);
        close(server_socket_);
        server_socket_ = -1;
    }
}


// Getters e Estatísticas

int SimpleChatServer::get_client_count() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_.size();
}

std::vector<std::string> SimpleChatServer::get_connected_usernames() const {
    std::vector<std::string> usernames;
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (const auto& pair : clients_) {
        usernames.push_back(pair.first);
    }
    return usernames;
}


void SimpleChatServer::print_stats() const {
    std::cout << "\n--- ESTATÍSTICAS DO SERVIDOR ---\n";
    std::cout << "Status: " << (running_.load() ? "Online" : "Offline") << "\n";
    std::cout << "Porta: " << port_ << "\n";
    std::cout << "Clientes conectados: " << get_client_count() << "\n";
    std::cout << "Total de conexões desde o início: " << total_connections_.load() << "\n";
    std::cout << "Total de mensagens processadas: " << total_messages_.load() << "\n";
    std::cout << "---------------------------------\n" << std::endl;
}

} 



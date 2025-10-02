#include "simple_chat_server.h"
#include "libtslog.h"
#include <cstring>      
#include <unistd.h>     
#include <sys/socket.h> 
namespace chat {

// Implementação da Classe ConnectedClient


ConnectedClient::ConnectedClient(int socket, const std::string& username) 
    : socket_fd_(socket), username_(username), active_(true) {
    LOG_INFO("Cliente criado: " + username + " (socket: " + std::to_string(socket) + ")");
}

ConnectedClient::~ConnectedClient() {
    disconnect();
    LOG_DEBUG("Cliente destruído: " + username_);
}

void ConnectedClient::queue_message(const Message& msg) {
    if (!active_.load()) {
        return;
    }
    try {
        outgoing_messages_.push(msg);
    } catch (const std::exception& e) {
        LOG_ERROR("Falha ao enfileirar mensagem para " + username_ + ": " + e.what());
        disconnect();
    }
}

void ConnectedClient::disconnect() {
    if (active_.exchange(false)) {
        LOG_INFO("Desconectando cliente: " + username_);
        outgoing_messages_.shutdown();
        
        if (socket_fd_ != -1) {
            shutdown(socket_fd_, SHUT_RDWR);
            close(socket_fd_);
            socket_fd_ = -1;
        }

        if (sender_thread_.joinable()) {
            sender_thread_.join();
        }
    }
}

void ConnectedClient::start_sender_thread() {
    sender_thread_ = std::thread(&ConnectedClient::sender_thread_func, this);
}

void ConnectedClient::sender_thread_func() {
    LOG_DEBUG("Thread de envio iniciada para " + username_);
    
    while (active_.load()) {
        try {
            auto msg_opt = outgoing_messages_.pop_timeout(std::chrono::milliseconds(500));
            if (msg_opt.has_value()) {
                if (!send_message_direct(msg_opt.value())) {
                    LOG_WARNING("Falha ao enviar mensagem para " + username_ + ", fechando conexão.");
                    active_.store(false);
                }
            }
        } catch (const std::runtime_error&) {
            
            break; 
        }
    }
    LOG_DEBUG("Thread de envio finalizada para " + username_);
}

bool ConnectedClient::send_message_direct(const Message& msg) {
    if (!active_.load() || socket_fd_ == -1) {
        return false;
    }
    
    std::string serialized = msg.serialize() + "\n";
    ssize_t sent = send(socket_fd_, serialized.c_str(), serialized.length(), MSG_NOSIGNAL);
    
    if (sent <= 0) {
        LOG_WARNING("Falha ao enviar dados para " + username_ + " (errno: " + strerror(errno) + ")");
        return false;
    }
    return true;
}

} 

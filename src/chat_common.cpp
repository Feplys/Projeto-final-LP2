#include "chat_common.h"
#include <chrono>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <arpa/inet.h>
#include <sys/socket.h>

namespace chat {

// Implementação da struct Message
Message::Message() : type(MessageType::CHAT) {
    memset(username, 0, MAX_USERNAME_SIZE);
    memset(content, 0, MAX_MESSAGE_SIZE);
}

Message::Message(MessageType t, const std::string& user, const std::string& msg)
    : type(t) {
    memset(username, 0, MAX_USERNAME_SIZE);
    memset(content, 0, MAX_MESSAGE_SIZE);
    strncpy(username, user.c_str(), MAX_USERNAME_SIZE - 1);
    strncpy(content, msg.c_str(), MAX_MESSAGE_SIZE - 1);
}

std::string Message::serialize() const {
    std::stringstream ss;
    ss << static_cast<int>(type) << "|" << username << "|" << content;
    return ss.str();
}

Message Message::deserialize(const std::string& data) {
    Message msg;
    std::stringstream ss(data);
    std::string item;
    
    try {
        if (std::getline(ss, item, '|')) {
            msg.type = static_cast<MessageType>(std::stoi(item));
        }
        if (std::getline(ss, item, '|')) {
            strncpy(msg.username, item.c_str(), MAX_USERNAME_SIZE - 1);
        }
        // O resto da linha é o conteúdo
        std::getline(ss, item);
        strncpy(msg.content, item.c_str(), MAX_MESSAGE_SIZE - 1);

    } catch (const std::exception&) {
        msg.type = MessageType::ERROR_MSG;
        strcpy(msg.content, "Mensagem corrompida recebida.");
    }
    return msg;
}

bool Message::is_valid() const {
    return (type >= MessageType::CONNECT && type <= MessageType::ERROR_MSG);
}

std::string Message::to_string() const {
    std::stringstream ss;
    ss << "Message{type=" << static_cast<int>(type)
       << ", user='" << username << "'"
       << ", content='" << content << "'}";
    return ss.str();
}

// Implementação de NetworkUtils
bool NetworkUtils::is_valid_port(int port) {
    return port > 1024 && port <= 65535;
}

bool NetworkUtils::is_valid_ip(const std::string& ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) != 0;
}

std::string NetworkUtils::get_timestamp_str() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    return ss.str();
}

std::string NetworkUtils::format_message_for_display(const Message& msg) {
    std::stringstream ss;
    switch (msg.type) {
        case MessageType::CONNECT:
            ss << "*** " << msg.username << " entrou no chat ***";
            break;
        case MessageType::DISCONNECT:
            ss << "*** " << msg.username << " saiu do chat ***";
            break;
        case MessageType::CHAT:
            ss << "[" << get_timestamp_str() << "] "
               << msg.username << ": " << msg.content;
            break;
        case MessageType::SERVER_MSG:
            ss << ">>> SERVIDOR: " << msg.content;
            break;
        case MessageType::ERROR_MSG:
            ss << "!!! ERRO: " << msg.content;
            break;
    }
    return ss.str();
}

} 



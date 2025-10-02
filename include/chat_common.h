#ifndef CHAT_COMMON_H
#define CHAT_COMMON_H

#include <string>
#include <cstdint>

namespace chat {

// Constantes do protocolo
const int DEFAULT_PORT = 8080;
const int MAX_MESSAGE_SIZE = 512;
const int MAX_USERNAME_SIZE = 32;
const int MAX_CLIENTS = 50; 
const int BUFFER_SIZE = 1024;

// Tipos de mensagem do protocolo
enum class MessageType : uint8_t {
    CONNECT = 1,
    DISCONNECT = 2,
    CHAT = 3,
    SERVER_MSG = 4,
    ERROR_MSG = 5 
};

// Estrutura da mensagem
struct Message {
    MessageType type;
    char username[MAX_USERNAME_SIZE];
    char content[MAX_MESSAGE_SIZE];

    Message();
    Message(MessageType t, const std::string& user, const std::string& msg);

    std::string serialize() const;
    static Message deserialize(const std::string& data);
    bool is_valid() const;
    std::string to_string() const;
};

// Utilitários de rede
class NetworkUtils {
public:
    static bool is_valid_port(int port);
    static bool is_valid_ip(const std::string& ip);
    static std::string get_timestamp_str();
    static std::string format_message_for_display(const Message& msg);
};

// Status da conexão
enum class ConnectionStatus {
    DISCONNECTED = 0,
    CONNECTING = 1,
    CONNECTED = 2,
    ERROR_STATE = 3
};

} 

#endif 



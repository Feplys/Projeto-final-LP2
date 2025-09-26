#ifndef CHAT_COMMON_H
#define CHAT_COMMON_H

#include <string>
#include <cstdint>

namespace chat {

// Constantes do sistema
const int DEFAULT_PORT = 8080;
const int MAX_CLIENTS = 100;

// Constantes do protocolo
const int MAX_MESSAGE_SIZE = 1024;
const int MAX_USERNAME_SIZE = 32;

// Tipos de mensagem do protocolo
enum class MessageType : uint8_t {
    JOIN = 1,
    LEAVE = 2,
    CHAT = 3,
    PRIVATE = 4,
    USER_LIST = 5,
    ERROR_MSG = 6,
    SUCCESS = 7
};

} // namespace chat

#endif // CHAT_COMMON_H

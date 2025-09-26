#ifndef CHAT_EXCEPTIONS_H
#define CHAT_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace chat {

// Exceção base para todo o sistema de chat
class ChatException : public std::runtime_error {
public:
    explicit ChatException(const std::string& message) : std::runtime_error(message) {}
};

// Exceções relacionadas ao Logger
class LoggerException : public ChatException {
public:
    explicit LoggerException(const std::string& message) : ChatException(message) {}
};

// Exceções de rede (para etapas futuras)
class NetworkException : public ChatException {
public:
    explicit NetworkException(const std::string& message) : ChatException(message) {}
};

class SocketException : public NetworkException {
public:
    explicit SocketException(const std::string& message) : NetworkException(message) {}
};

} // namespace chat

#endif // CHAT_EXCEPTIONS_H

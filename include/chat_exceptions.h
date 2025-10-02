#ifndef CHAT_EXCEPTIONS_H
#define CHAT_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace chat {


class ChatException : public std::runtime_error {
public:
    ChatException(const std::string& message) : std::runtime_error(message) {}
    virtual ~ChatException() = default;
    
   
    virtual std::string get_suggestion() const {
        return "Verifique a documentação ou contate o suporte";
    }
};

class LoggerException : public ChatException {
public:
    LoggerException(const std::string& message) : ChatException(message) {}
    
    std::string get_suggestion() const override {
        return "Verifique permissões do diretório de logs e espaço em disco.";
    }
};


class NetworkException : public ChatException {
public:
    NetworkException(const std::string& message) : ChatException(message) {}
    
    std::string get_suggestion() const override {
        return "Verifique a conexão, o endereço do servidor e configurações de firewall.";
    }
};

} 

#endif 



#ifndef CHAT_COMMON_H
#define CHAT_COMMON_H

#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>

namespace chat {

// --- CONSTANTES ---
const int DEFAULT_PORT = 8080;
const int MAX_CLIENTS = 50;
const int BUFFER_SIZE = 4096;
const int MAX_USERNAME_SIZE = 16;
const int MAX_PASSWORD_SIZE = 16;
const int MAX_CONTENT_SIZE = 512;

// --- TIPOS DE MENSAGEM ---
enum class MessageType : uint8_t {
    REGISTER_REQUEST, LOGIN_REQUEST, DISCONNECT_REQUEST,
    CHAT_BROADCAST, PRIVATE_MESSAGE, AUTH_SUCCESS,
    AUTH_FAILURE, SERVER_MESSAGE, ERROR_MSG,
};

// --- ESTRUTURA DA MENSAGEM ---
struct Message {
    MessageType type;
    char username[MAX_USERNAME_SIZE];
    char password[MAX_PASSWORD_SIZE];
    char target_user[MAX_USERNAME_SIZE];
    char content[MAX_CONTENT_SIZE];

    Message(); 
    Message(MessageType type, const std::string& user, const std::string& content);
    
    std::string serialize() const;
    static Message deserialize(const std::string& data);
};

// --- CLASSE DE UTILITÁRIOS ---
class Utils {
public:
    static std::string get_timestamp_str();
    static bool is_valid_username(const std::string& name);
    static bool is_valid_password(const std::string& pass);
    static std::string filter_profanity(const std::string& message);
    // Nova função de leitura segura que recebe o seu próprio buffer
    static std::string read_line(int socket_fd, std::string& buffer);
};

} 

#endif 



#include "chat_common.h"
#include "libtslog.h"
#include <sys/socket.h>
#include <unistd.h>

namespace chat {

// Construtores
Message::Message() {
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    memset(target_user, 0, sizeof(target_user));
    memset(content, 0, sizeof(content));
}

Message::Message(MessageType msg_type, const std::string& user, const std::string& msg_content) : Message() {
    type = msg_type;
    strncpy(username, user.c_str(), MAX_USERNAME_SIZE - 1);
    strncpy(content, msg_content.c_str(), MAX_CONTENT_SIZE - 1);
}

// Funções de Serialização
std::string Message::serialize() const {
    const char DELIMITER = '|';
    std::stringstream ss;
    ss << static_cast<int>(type) << DELIMITER << username << DELIMITER
       << password << DELIMITER << target_user << DELIMITER << content;
    return ss.str();
}

Message Message::deserialize(const std::string& data) {
    Message msg;
    std::stringstream ss(data);
    std::string temp;
    const char DELIMITER = '|';
    try {
        if (std::getline(ss, temp, DELIMITER)) msg.type = static_cast<MessageType>(std::stoi(temp));
        if (std::getline(ss, temp, DELIMITER)) strncpy(msg.username, temp.c_str(), MAX_USERNAME_SIZE - 1);
        if (std::getline(ss, temp, DELIMITER)) strncpy(msg.password, temp.c_str(), MAX_PASSWORD_SIZE - 1);
        if (std::getline(ss, temp, DELIMITER)) strncpy(msg.target_user, temp.c_str(), MAX_USERNAME_SIZE - 1);
        if (std::getline(ss, temp)) strncpy(msg.content, temp.c_str(), MAX_CONTENT_SIZE - 1);
    } catch (...) {
        msg.type = MessageType::ERROR_MSG;
    }
    return msg;
}

// --- CLASSE DE UTILITÁRIOS ---

std::string Utils::read_line(int socket_fd, std::string& buffer) {
    char read_char;
    while (true) {
        size_t pos = buffer.find('\n');
        if (pos != std::string::npos) {
            std::string line = buffer.substr(0, pos);
            buffer.erase(0, pos + 1);
            return line;
        }
        ssize_t bytes_read = recv(socket_fd, &read_char, 1, 0);
        if (bytes_read <= 0) return "";
        buffer += read_char;
    }
}

std::string Utils::get_timestamp_str() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    return ss.str();
}

bool Utils::is_valid_username(const std::string& name) {
    if (name.length() < 3 || name.length() >= MAX_USERNAME_SIZE) return false;
    return std::all_of(name.begin(), name.end(), [](char c){ return std::isalnum(c); });
}

bool Utils::is_valid_password(const std::string& pass) {
    return pass.length() >= 4 && pass.length() < MAX_PASSWORD_SIZE;
}

std::string Utils::filter_profanity(const std::string& message) {
    std::string lower_message = message;
    std::transform(lower_message.begin(), lower_message.end(), lower_message.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    const std::vector<std::string> bad_words = {"palavraofeia", "chulao", "improprio"};
    std::string result = message;
    for (const auto& word : bad_words) {
        size_t pos = lower_message.find(word);
        while (pos != std::string::npos) {
            result.replace(pos, word.length(), std::string(word.length(), '*'));
            pos = lower_message.find(word, pos + 1);
        }
    }
    return result;
}

} // namespace chat



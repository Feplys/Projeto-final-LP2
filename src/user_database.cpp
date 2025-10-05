#include "user_database.h"
#include "libtslog.h"
#include <fstream>
#include <sstream>

namespace chat {

UserDatabase::UserDatabase(const std::string& filepath) : db_filepath_(filepath) {
    load();
}

void UserDatabase::load() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    std::ifstream file(db_filepath_);
    if (!file.is_open()) {
        LOG_WARNING("Arquivo de banco de dados '" + db_filepath_ + "' não encontrado. Será criado um novo.");
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string username, password;
        if (std::getline(ss, username, ':') && std::getline(ss, password)) {
            users_[username] = password;
        }
    }
    LOG_INFO(std::to_string(users_.size()) + " usuários carregados do banco de dados.");
}

void UserDatabase::save() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    std::ofstream file(db_filepath_, std::ios::trunc);
    if (!file.is_open()) {
        LOG_ERROR("Não foi possível salvar o banco de dados em '" + db_filepath_ + "'");
        return;
    }

    for (const auto& pair : users_) {
        file << pair.first << ":" << pair.second << "\n";
    }
}

bool UserDatabase::add_user(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    if (users_.count(username)) {
        return false; // Usuário já existe
    }
    users_[username] = password; // Nota: Senha em texto plano para simplicidade
    save();
    LOG_INFO("Novo usuário '" + username + "' registrado.");
    return true;
}

bool UserDatabase::validate_user(const std::string& username, const std::string& password) const {
    std::lock_guard<std::mutex> lock(db_mutex_);
    if (users_.count(username)) {
        return users_.at(username) == password;
    }
    return false;
}

bool UserDatabase::user_exists(const std::string& username) const {
    std::lock_guard<std::mutex> lock(db_mutex_);
    return users_.count(username);
}

size_t UserDatabase::get_user_count() const {
    std::lock_guard<std::mutex> lock(db_mutex_);
    return users_.size();
}

} // namespace chat



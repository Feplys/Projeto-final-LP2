#ifndef USER_DATABASE_H
#define USER_DATABASE_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>

namespace chat {

class UserDatabase {
private:
    std::string db_filepath_;
    std::unordered_map<std::string, std::string> users_; // username -> password
    mutable std::mutex db_mutex_;

    void load();
    void save();

public:
    UserDatabase(const std::string& filepath = "users.db");
    
    bool add_user(const std::string& username, const std::string& password);
    bool validate_user(const std::string& username, const std::string& password) const;
    bool user_exists(const std::string& username) const;
    size_t get_user_count() const;
};

} 

#endif 



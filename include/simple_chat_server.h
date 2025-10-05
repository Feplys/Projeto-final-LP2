#ifndef SIMPLE_CHAT_SERVER_H
#define SIMPLE_CHAT_SERVER_H

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <unordered_map>

#include "chat_common.h"
#include "user_database.h"
#include "connected_client.h"

namespace chat {

class SimpleChatServer {
private:
    int server_socket_;
    int port_;
    std::atomic<bool> running_;
    
    std::thread accept_thread_; 

    UserDatabase user_db_; 

    mutable std::mutex online_users_mutex_;
    std::unordered_map<std::string, std::shared_ptr<ConnectedClient>> online_users_;

    std::atomic<int> total_connections_;
    std::atomic<long> total_messages_processed_;

    bool setup_server_socket();
    void cleanup_server_socket();
    
    void accept_connections();
    void handle_client(int client_socket, std::string client_addr);
    
    void process_client_message(const Message& msg, std::shared_ptr<ConnectedClient> client);
    void add_online_user(const std::string& username, std::shared_ptr<ConnectedClient> client);
    void remove_online_user(const std::string& username);
    
    void broadcast_message(const Message& msg);
    void send_private_message(const Message& msg);

public:
    SimpleChatServer(int port = DEFAULT_PORT);
    ~SimpleChatServer();

    bool start();
    void stop();

    bool is_running() const { return running_.load(); }
    int get_online_user_count() const;
    std::vector<std::string> get_online_usernames() const;
    void print_stats() const;
};

} // namespace chat

#endif // SIMPLE_CHAT_SERVER_H



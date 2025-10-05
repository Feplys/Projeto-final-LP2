#ifndef SIMPLE_CHAT_CLIENT_H
#define SIMPLE_CHAT_CLIENT_H

#include <string>
#include <thread>
#include <atomic>
#include "chat_common.h"

namespace chat {

class SimpleChatClient {
private:
    int socket_fd_;
    std::atomic<bool> is_connected_;
    std::atomic<bool> is_authenticated_;
    std::string username_;
    std::string server_address_;
    int server_port_;
    
    std::thread receiver_thread_;
    std::atomic<bool> should_stop_;

    void cleanup_connection();
    void receiver_thread_func();
    void process_chat_message(const Message& msg);
    
    bool establish_connection(); 
    bool handle_auth_response(const std::string& response_data, const std::string& original_username);
    
    bool send_data(const std::string& data);

public:
    SimpleChatClient(const std::string& server_addr = "127.0.0.1", int port = DEFAULT_PORT);
    ~SimpleChatClient();

    bool connect_and_login(const std::string& username, const std::string& password);
    bool connect_and_register(const std::string& username, const std::string& password);
    void disconnect();

    void send_broadcast(const std::string& message);
    void send_private(const std::string& target, const std::string& message);

    bool is_authenticated() const { return is_authenticated_.load(); }
    const std::string& get_username() const { return username_; }
};

} 

#endif 



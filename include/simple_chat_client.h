#ifndef SIMPLE_CHAT_CLIENT_H
#define SIMPLE_CHAT_CLIENT_H

#include "chat_common.h"
#include "chat_exceptions.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace chat {

class SimpleChatClient {
private:
    int socket_fd_;
    std::atomic<ConnectionStatus> status_;
    std::string username_;
    std::string server_address_;
    int server_port_;

    std::thread receiver_thread_;
    std::atomic<bool> should_stop_;

    std::atomic<int> messages_sent_;
    std::atomic<int> messages_received_;

    // Sincronização (Variável que faltava)
    mutable std::mutex connection_mutex_;

public:
    SimpleChatClient(const std::string& server_addr = "127.0.0.1",
                     int port = DEFAULT_PORT);
    ~SimpleChatClient();

    bool connect_to_server(const std::string& username);
    void disconnect();

    bool is_connected() const;
    ConnectionStatus get_status() const { return status_.load(); }
    const std::string& get_username() const { return username_; }

    bool send_message(const std::string& message);

    // Estatísticas (Função que faltava)
    void print_stats() const;
    int get_messages_sent() const { return messages_sent_.load(); }
    int get_messages_received() const { return messages_received_.load(); }

    void set_auto_mode(bool enabled) { auto_mode_ = enabled; }

private:
    bool auto_mode_ = false;

    bool setup_connection();
    void cleanup_connection();
    void receiver_thread_func();
    std::string receive_string_from_server();
    bool send_string_to_server(const std::string& data);
    void process_received_message(const Message& msg);
    void handle_server_message(const Message& msg);
    void display_message(const Message& msg);
    void update_status(ConnectionStatus new_status);
};

} 

#endif 



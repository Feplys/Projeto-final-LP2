#ifndef SIMPLE_CHAT_SERVER_H
#define SIMPLE_CHAT_SERVER_H

#include "chat_common.h"
#include "thread_safe_queue.h"
#include "chat_exceptions.h"
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace chat {

class ConnectedClient {
private:
    int socket_fd_;
    std::string username_;
    std::atomic<bool> active_;
    std::thread sender_thread_;
    ThreadSafeQueue<Message> outgoing_messages_;

public:
    ConnectedClient(int socket, const std::string& username);
    ~ConnectedClient();

    int get_socket() const { return socket_fd_; }
    const std::string& get_username() const { return username_; }
    bool is_active() const { return active_.load(); }

    void queue_message(const Message& msg);
    void disconnect();
    void start_sender_thread();

private:
    void sender_thread_func();
    bool send_message_direct(const Message& msg);
};

class SimpleChatServer {
private:
    int server_socket_;
    std::atomic<bool> running_;
    int port_;

    mutable std::mutex clients_mutex_;
    std::unordered_map<std::string, std::shared_ptr<ConnectedClient>> clients_;
    std::vector<std::thread> client_threads_;

    std::thread accept_thread_;

    std::atomic<int> total_connections_;
    std::atomic<int> total_messages_;

public:
    SimpleChatServer(int port = DEFAULT_PORT);
    ~SimpleChatServer();

    bool start();
    void stop();
    bool is_running() const { return running_.load(); }

    void print_stats() const;
    int get_client_count() const;
    std::vector<std::string> get_connected_usernames() const;

private:
    bool setup_server_socket();
    void cleanup_server_socket();
    void accept_connections();
    void handle_client(int client_socket, const std::string& client_addr);
    void process_client_message(const Message& msg, std::shared_ptr<ConnectedClient> client);
    void broadcast_message(const Message& msg, const std::string& exclude_user = "");
    bool add_client(const std::string& username, std::shared_ptr<ConnectedClient> client);
    void remove_client(const std::string& username);
    std::string receive_string_from_socket(int socket_fd);
    bool send_string_to_socket(int socket_fd, const std::string& data);
};

} 

#endif 



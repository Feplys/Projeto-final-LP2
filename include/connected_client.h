#ifndef CONNECTED_CLIENT_H
#define CONNECTED_CLIENT_H

#include "chat_common.h"
#include "thread_safe_queue.h"
#include <string>
#include <thread>
#include <atomic>
#include <memory>

namespace chat {

class ConnectedClient {
private:
    int socket_fd_;
    std::string username_;
    std::atomic<bool> active_;
    std::thread sender_thread_;
    ThreadSafeQueue<Message> outgoing_messages_;

    void sender_thread_func();
    bool send_message_direct(const Message& msg);

public:
    ConnectedClient(int socket, const std::string& username);
    ~ConnectedClient();

    bool is_active() const { return active_.load(); }
    void queue_message(const Message& msg);
    void disconnect();
    void start_sender_thread();
    std::string receive_data_blocking(std::string& read_buffer); 
};

} // namespace chat

#endif // CONNECTED_CLIENT_H



#include "connected_client.h"
#include "libtslog.h"
#include <sys/socket.h>
#include <unistd.h>

namespace chat {

ConnectedClient::ConnectedClient(int socket, const std::string& username)
    : socket_fd_(socket), username_(username), active_(true) {}

ConnectedClient::~ConnectedClient() {
    disconnect();
}

void ConnectedClient::queue_message(const Message& msg) {
    if (active_.load()) outgoing_messages_.push(msg);
}

void ConnectedClient::disconnect() {
    if (active_.exchange(false)) {
        outgoing_messages_.shutdown();
        if (socket_fd_ != -1) {
            shutdown(socket_fd_, SHUT_RDWR);
            close(socket_fd_);
            socket_fd_ = -1;
        }
        if (sender_thread_.joinable()) sender_thread_.join();
    }
}

void ConnectedClient::start_sender_thread() {
    sender_thread_ = std::thread(&ConnectedClient::sender_thread_func, this);
}

void ConnectedClient::sender_thread_func() {
    while (active_.load()) {
        try {
            auto msg_opt = outgoing_messages_.pop_timeout(std::chrono::seconds(1));
            if (msg_opt.has_value() && !send_message_direct(msg_opt.value())) break;
        } catch (...) { break; }
    }
    active_.store(false);
}

bool ConnectedClient::send_message_direct(const Message& msg) {
    if (!active_.load()) return false;
    std::string data = msg.serialize() + "\n";
    return send(socket_fd_, data.c_str(), data.length(), MSG_NOSIGNAL) > 0;
}

std::string ConnectedClient::receive_data_blocking(std::string& read_buffer) {
    return Utils::read_line(socket_fd_, read_buffer);
}

} // namespace chat



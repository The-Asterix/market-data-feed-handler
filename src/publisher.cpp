#include "network.h"
#include "protocol.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>

std::vector<uint8_t> read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) throw std::runtime_error("Could not open file: " + path);
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    in.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

// Tells us how many bytes make up "one message" so we send exactly
// one message per write() call, not a random chunk.
size_t message_size_for_type(char type) {
    if (type == 'A') return sizeof(AddOrderMessage);
    if (type == 'D') return sizeof(DeleteOrderMessage);
    if (type == 'E') return sizeof(OrderExecutedMessage);
    return 0;
}

int main() {
    const int PORT = 9000;

    int server_fd = create_server_socket(PORT);
    if (server_fd < 0) return 1;

    std::cout << "Publisher listening on port " << PORT << "...\n";
    int client_fd = accept_client(server_fd);
    if (client_fd < 0) return 1;

    auto data = read_file("data/sample_feed.bin");

    size_t offset = 0;
    while (offset < data.size()) {
        char type = static_cast<char>(data[offset]);
        size_t msg_size = message_size_for_type(type);
        if (msg_size == 0 || offset + msg_size > data.size()) break;

        ssize_t sent = write(client_fd, data.data() + offset, msg_size);
        if (sent <= 0) {
            std::cerr << "Send failed, client likely disconnected\n";
            break;
        }

        std::cout << "Sent message type '" << type << "' (" << msg_size << " bytes)\n";
        offset += msg_size;

        // Simulate a live feed trickling in, instead of dumping everything instantly.
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    std::cout << "All messages sent. Closing connection.\n";
    close(client_fd);
    close(server_fd);
    return 0;
}

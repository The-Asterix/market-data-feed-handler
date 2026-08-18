#include "network.h"
#include "parser.h"
#include "order_book.h"
#include "thread_safe_queue.h"
#include <iostream>
#include <unistd.h>
#include <vector>
#include <thread>

std::string symbol_to_string(const char* sym) {
    return std::string(sym, 8);
}

void print_message(const ParsedMessage& msg) {
    switch (msg.type) {
        case MessageType::Add:
            std::cout << "[ADD]    order_id=" << msg.add.order_id
                      << " side=" << msg.add.side
                      << " price=" << msg.add.price
                      << " qty=" << msg.add.quantity
                      << " symbol=" << symbol_to_string(msg.add.symbol) << "\n";
            break;
        case MessageType::Delete:
            std::cout << "[DELETE] order_id=" << msg.del.order_id << "\n";
            break;
        case MessageType::Execute:
            std::cout << "[EXEC]   order_id=" << msg.exec.order_id
                      << " executed_qty=" << msg.exec.executed_quantity << "\n";
            break;
        default:
            std::cout << "[UNKNOWN]\n";
    }
}

// PRODUCER: only job is to read raw bytes off the socket as fast as
// possible and hand each chunk to the queue. It does NOT parse anything --
// that's the whole point, so a slow parse never blocks the network read.
void network_thread_func(int sock_fd, ThreadSafeQueue<std::vector<uint8_t>>& queue) {
    uint8_t chunk[1024];
    while (true) {
        ssize_t n = read(sock_fd, chunk, sizeof(chunk));
        if (n <= 0) {
            std::cout << "[network thread] Connection closed.\n";
            break;
        }
        // Copy this chunk into a vector and push it -- push() takes
        // ownership via std::move so we're not copying twice.
        queue.push(std::vector<uint8_t>(chunk, chunk + n));
    }
    // No more data will ever come -- tell the consumer to stop waiting
    // once it's drained whatever's left in the queue.
    queue.shutdown();
}

// CONSUMER: pulls raw byte chunks off the queue, reassembles them into
// complete messages (same incremental-parsing logic as before), and
// updates the order book. This runs independently of network timing.
void parser_thread_func(ThreadSafeQueue<std::vector<uint8_t>>& queue) {
    OrderBook book;
    std::vector<uint8_t> buffer; // leftover partial message across chunks

    std::vector<uint8_t> chunk;
    while (queue.wait_and_pop(chunk)) {
        buffer.insert(buffer.end(), chunk.begin(), chunk.end());

        size_t offset = 0;
        while (offset < buffer.size()) {
            ParsedMessage msg;
            size_t consumed = FeedParser::parse_one(buffer.data() + offset, buffer.size() - offset, msg);
            if (consumed == 0) break; // incomplete message, wait for more bytes

            print_message(msg);
            switch (msg.type) {
                case MessageType::Add:     book.apply_add(msg.add); break;
                case MessageType::Delete:  book.apply_delete(msg.del); break;
                case MessageType::Execute: book.apply_execute(msg.exec); break;
                default: break;
            }
            book.print_top_of_book();

            offset += consumed;
        }

        buffer.erase(buffer.begin(), buffer.begin() + offset);
    }
    std::cout << "[parser thread] Done. Queue drained and shut down.\n";
}

int main() {
    const std::string HOST = "127.0.0.1";
    const int PORT = 9000;

    int sock_fd = connect_to_server(HOST, PORT);
    if (sock_fd < 0) return 1;
    std::cout << "Connected to publisher at " << HOST << ":" << PORT << "\n";

    ThreadSafeQueue<std::vector<uint8_t>> queue;

    // Launch both threads. std::thread starts running the function
    // immediately, in parallel with main().
    std::thread network_thread(network_thread_func, sock_fd, std::ref(queue));
    std::thread parser_thread(parser_thread_func, std::ref(queue));

    // join() blocks main() here until that thread finishes.
    // We must join both before main() exits, or the program can crash
    // (destroying thread objects that are still running is undefined behavior).
    network_thread.join();
    parser_thread.join();

    close(sock_fd);
    return 0;
}

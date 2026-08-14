#include "network.h"
#include "parser.h"
#include "order_book.h"
#include <iostream>
#include <unistd.h>
#include <vector>

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

int main() {
    const std::string HOST = "127.0.0.1";
    const int PORT = 9000;

    int sock_fd = connect_to_server(HOST, PORT);
    if (sock_fd < 0) return 1;

    std::cout << "Connected to publisher at " << HOST << ":" << PORT << "\n";

    OrderBook book;
    std::vector<uint8_t> buffer; // holds bytes not yet parsed into a full message
    uint8_t chunk[1024];

    while (true) {
        ssize_t n = read(sock_fd, chunk, sizeof(chunk));
        if (n <= 0) {
            std::cout << "Connection closed by publisher.\n";
            break;
        }

        buffer.insert(buffer.end(), chunk, chunk + n);

        // Parse as many complete messages as we can from the buffer.
        // A message might be split across two reads -- if so, parse_one
        // returns 0 and we simply wait for the next read to complete it.
        size_t offset = 0;
        while (offset < buffer.size()) {
            ParsedMessage msg;
            size_t consumed = FeedParser::parse_one(buffer.data() + offset, buffer.size() - offset, msg);
            if (consumed == 0) break;

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

        // Drop what we consumed, keep any leftover partial message for next read.
        buffer.erase(buffer.begin(), buffer.begin() + offset);
    }

    close(sock_fd);
    return 0;
}

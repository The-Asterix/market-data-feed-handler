#include "parser.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>

std::vector<uint8_t> read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) throw std::runtime_error("Could not open file: " + path);
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    in.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

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
    try {
        auto data = read_file("data/sample_feed.bin");
        auto messages = FeedParser::parse_all(data.data(), data.size());

        std::cout << "Parsed " << messages.size() << " messages:\n\n";
        for (const auto& msg : messages) {
            print_message(msg);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

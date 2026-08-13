#include "protocol.h"
#include <fstream>
#include <iostream>
#include <cstring>

void write_symbol(char* dest, const char* sym) {
    std::memset(dest, ' ', 8);
    std::memcpy(dest, sym, std::min(std::strlen(sym), size_t(8)));
}

int main() {
    std::ofstream out("data/sample_feed.bin", std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open output file. Did you run this from the project root?\n";
        return 1;
    }

    uint64_t ts = 34200000000000ULL; // arbitrary starting timestamp
    uint64_t order_id = 1000;

    // Add order: buy 100 AAPL @ 150.25
    {
        AddOrderMessage msg{};
        msg.message_type = 'A';
        msg.timestamp = ts++;
        msg.order_id = order_id++;
        msg.side = 'B';
        msg.price = 1502500;
        msg.quantity = 100;
        write_symbol(msg.symbol, "AAPL");
        out.write(reinterpret_cast<char*>(&msg), sizeof(msg));
    }

    // Add order: sell 50 AAPL @ 150.30
    {
        AddOrderMessage msg{};
        msg.message_type = 'A';
        msg.timestamp = ts++;
        msg.order_id = order_id++;
        msg.side = 'S';
        msg.price = 1503000;
        msg.quantity = 50;
        write_symbol(msg.symbol, "AAPL");
        out.write(reinterpret_cast<char*>(&msg), sizeof(msg));
    }

    // Order executed: order 1000 partially filled, 40 shares
    {
        OrderExecutedMessage msg{};
        msg.message_type = 'E';
        msg.timestamp = ts++;
        msg.order_id = 1000;
        msg.executed_quantity = 40;
        out.write(reinterpret_cast<char*>(&msg), sizeof(msg));
    }

    // Delete order 1001
    {
        DeleteOrderMessage msg{};
        msg.message_type = 'D';
        msg.timestamp = ts++;
        msg.order_id = 1001;
        out.write(reinterpret_cast<char*>(&msg), sizeof(msg));
    }

    out.close();
    std::cout << "Sample feed written to data/sample_feed.bin (4 messages)\n";
    return 0;
}

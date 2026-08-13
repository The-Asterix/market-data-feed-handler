#include "parser.h"
#include <cstring>

size_t FeedParser::parse_one(const uint8_t* data, size_t remaining, ParsedMessage& out) {
    if (remaining < 1) return 0;

    char type = static_cast<char>(data[0]);

    if (type == 'A') {
        if (remaining < sizeof(AddOrderMessage)) return 0;
        std::memcpy(&out.add, data, sizeof(AddOrderMessage));
        out.type = MessageType::Add;
        return sizeof(AddOrderMessage);
    }
    if (type == 'D') {
        if (remaining < sizeof(DeleteOrderMessage)) return 0;
        std::memcpy(&out.del, data, sizeof(DeleteOrderMessage));
        out.type = MessageType::Delete;
        return sizeof(DeleteOrderMessage);
    }
    if (type == 'E') {
        if (remaining < sizeof(OrderExecutedMessage)) return 0;
        std::memcpy(&out.exec, data, sizeof(OrderExecutedMessage));
        out.type = MessageType::Execute;
        return sizeof(OrderExecutedMessage);
    }

    out.type = MessageType::Unknown;
    return 0; // unrecognized type byte — stop parsing, likely corrupt data
}

std::vector<ParsedMessage> FeedParser::parse_all(const uint8_t* data, size_t size) {
    std::vector<ParsedMessage> messages;
    size_t offset = 0;

    while (offset < size) {
        ParsedMessage msg;
        size_t consumed = parse_one(data + offset, size - offset, msg);
        if (consumed == 0) break; // stop on unknown/corrupt/incomplete message
        messages.push_back(msg);
        offset += consumed;
    }

    return messages;
}

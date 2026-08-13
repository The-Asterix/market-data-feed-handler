#pragma once
#include "protocol.h"
#include <cstdint>
#include <cstddef>
#include <vector>

// Represents one decoded message, tagged by type so calling code
// knows which struct inside the union is valid.
enum class MessageType { Add, Delete, Execute, Unknown };

struct ParsedMessage {
    MessageType type;
    AddOrderMessage add;
    DeleteOrderMessage del;
    OrderExecutedMessage exec;
};

class FeedParser {
public:
    // Parses one message starting at `data`, returns bytes consumed.
    // Returns 0 if it doesn't recognize the message type (corrupt/end of data).
    static size_t parse_one(const uint8_t* data, size_t remaining, ParsedMessage& out);

    // Parses an entire buffer into a vector of messages.
    static std::vector<ParsedMessage> parse_all(const uint8_t* data, size_t size);
};

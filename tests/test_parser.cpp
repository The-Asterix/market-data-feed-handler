#include <catch2/catch_test_macros.hpp>
#include "parser.h"
#include <cstring>

// Helper: builds a raw AddOrderMessage as bytes, exactly like the real
// generator/publisher would produce.
std::vector<uint8_t> make_add_bytes(uint64_t order_id, char side, uint32_t price, uint32_t qty) {
    AddOrderMessage msg{};
    msg.message_type = 'A';
    msg.timestamp = 123;
    msg.order_id = order_id;
    msg.side = side;
    msg.price = price;
    msg.quantity = qty;
    std::memset(msg.symbol, ' ', 8);
    std::memcpy(msg.symbol, "AAPL", 4);

    const uint8_t* raw = reinterpret_cast<const uint8_t*>(&msg);
    return std::vector<uint8_t>(raw, raw + sizeof(AddOrderMessage));
}

TEST_CASE("parse_one correctly parses a single Add message", "[parser]") {
    auto bytes = make_add_bytes(42, 'B', 1500000, 100);

    ParsedMessage out;
    size_t consumed = FeedParser::parse_one(bytes.data(), bytes.size(), out);

    REQUIRE(consumed == sizeof(AddOrderMessage));
    REQUIRE(out.type == MessageType::Add);
    REQUIRE(out.add.order_id == 42);
    REQUIRE(out.add.side == 'B');
    REQUIRE(out.add.price == 1500000);
    REQUIRE(out.add.quantity == 100);
}

TEST_CASE("parse_one returns 0 on incomplete message (partial bytes)", "[parser]") {
    auto bytes = make_add_bytes(1, 'B', 1000, 10);
    bytes.resize(bytes.size() - 5); // simulate a truncated/split message

    ParsedMessage out;
    size_t consumed = FeedParser::parse_one(bytes.data(), bytes.size(), out);

    REQUIRE(consumed == 0); // must NOT crash, just report "not enough data yet"
}

TEST_CASE("parse_one returns 0 on unknown message type", "[parser]") {
    std::vector<uint8_t> bytes = {'Z', 0, 0, 0, 0}; // 'Z' is not A/D/E

    ParsedMessage out;
    size_t consumed = FeedParser::parse_one(bytes.data(), bytes.size(), out);

    REQUIRE(consumed == 0);
}

TEST_CASE("parse_all parses multiple back-to-back messages", "[parser]") {
    auto msg1 = make_add_bytes(1, 'B', 1000, 10);
    auto msg2 = make_add_bytes(2, 'S', 2000, 20);

    std::vector<uint8_t> combined;
    combined.insert(combined.end(), msg1.begin(), msg1.end());
    combined.insert(combined.end(), msg2.begin(), msg2.end());

    auto messages = FeedParser::parse_all(combined.data(), combined.size());

    REQUIRE(messages.size() == 2);
    REQUIRE(messages[0].add.order_id == 1);
    REQUIRE(messages[1].add.order_id == 2);
}

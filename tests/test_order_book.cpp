#include <catch2/catch_test_macros.hpp>
#include "order_book.h"

AddOrderMessage make_add(uint64_t id, char side, uint32_t price, uint32_t qty) {
    AddOrderMessage msg{};
    msg.message_type = 'A';
    msg.order_id = id;
    msg.side = side;
    msg.price = price;
    msg.quantity = qty;
    return msg;
}

TEST_CASE("best_bid/best_ask are empty on a fresh order book", "[order_book]") {
    OrderBook book;
    REQUIRE_FALSE(book.best_bid().has_value());
    REQUIRE_FALSE(book.best_ask().has_value());
}

TEST_CASE("adding a single buy order sets the best bid", "[order_book]") {
    OrderBook book;
    book.apply_add(make_add(1, 'B', 1500000, 100));

    REQUIRE(book.best_bid().has_value());
    REQUIRE(*book.best_bid() == 1500000);
    REQUIRE_FALSE(book.best_ask().has_value());
}

TEST_CASE("best bid is always the HIGHEST buy price", "[order_book]") {
    OrderBook book;
    book.apply_add(make_add(1, 'B', 1000000, 10));
    book.apply_add(make_add(2, 'B', 1500000, 10)); // higher -> should become best
    book.apply_add(make_add(3, 'B', 1200000, 10));

    REQUIRE(*book.best_bid() == 1500000);
}

TEST_CASE("best ask is always the LOWEST sell price", "[order_book]") {
    OrderBook book;
    book.apply_add(make_add(1, 'S', 2000000, 10));
    book.apply_add(make_add(2, 'S', 1800000, 10)); // lower -> should become best
    book.apply_add(make_add(3, 'S', 1900000, 10));

    REQUIRE(*book.best_ask() == 1800000);
}

TEST_CASE("deleting the only order at best price removes that level", "[order_book]") {
    OrderBook book;
    book.apply_add(make_add(1, 'B', 1500000, 10));

    DeleteOrderMessage del{};
    del.order_id = 1;
    book.apply_delete(del);

    REQUIRE_FALSE(book.best_bid().has_value());
}

TEST_CASE("full execution removes the order from the book", "[order_book]") {
    OrderBook book;
    book.apply_add(make_add(1, 'B', 1500000, 50));

    OrderExecutedMessage exec{};
    exec.order_id = 1;
    exec.executed_quantity = 50; // fully fills it
    book.apply_execute(exec);

    REQUIRE_FALSE(book.best_bid().has_value());
}

TEST_CASE("partial execution keeps the order resting at the same price", "[order_book]") {
    OrderBook book;
    book.apply_add(make_add(1, 'B', 1500000, 50));

    OrderExecutedMessage exec{};
    exec.order_id = 1;
    exec.executed_quantity = 20; // only partially fills
    book.apply_execute(exec);

    REQUIRE(book.best_bid().has_value());
    REQUIRE(*book.best_bid() == 1500000); // still resting, just smaller now
}

TEST_CASE("deleting an unknown order_id is safely ignored", "[order_book]") {
    OrderBook book;
    book.apply_add(make_add(1, 'B', 1500000, 10));

    DeleteOrderMessage del{};
    del.order_id = 999; // doesn't exist
    book.apply_delete(del);

    REQUIRE(book.best_bid().has_value()); // original order untouched
    REQUIRE(*book.best_bid() == 1500000);
}

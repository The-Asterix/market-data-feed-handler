#pragma once
#include "protocol.h"
#include <map>
#include <unordered_map>
#include <cstdint>
#include <optional>

struct RestingOrder {
    uint64_t order_id;
    char side;
    uint32_t price;
    uint32_t quantity; // remaining quantity (shrinks on partial execute)
};

class OrderBook {
public:
    void apply_add(const AddOrderMessage& msg);
    void apply_delete(const DeleteOrderMessage& msg);
    void apply_execute(const OrderExecutedMessage& msg);

    std::optional<uint32_t> best_bid() const;
    std::optional<uint32_t> best_ask() const;

    void print_top_of_book() const;

private:
    // bids: highest price first -> descending order
    std::map<uint32_t, std::vector<uint64_t>, std::greater<uint32_t>> bid_levels;
    // asks: lowest price first -> ascending order (map's default)
    std::map<uint32_t, std::vector<uint64_t>> ask_levels;

    // fast lookup from order_id -> full order details, so Delete/Execute
    // don't need to scan every price level
    std::unordered_map<uint64_t, RestingOrder> orders_by_id;

    void remove_order_from_level(const RestingOrder& order);
};

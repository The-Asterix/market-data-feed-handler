#include "order_book.h"
#include <iostream>
#include <algorithm>

void OrderBook::apply_add(const AddOrderMessage& msg) {
    RestingOrder order{msg.order_id, msg.side, msg.price, msg.quantity};
    orders_by_id[msg.order_id] = order;

    if (msg.side == 'B') {
        bid_levels[msg.price].push_back(msg.order_id);
    } else {
        ask_levels[msg.price].push_back(msg.order_id);
    }
}

void OrderBook::remove_order_from_level(const RestingOrder& order) {
    if (order.side == 'B') {
        auto& ids = bid_levels[order.price];
        ids.erase(std::remove(ids.begin(), ids.end(), order.order_id), ids.end());
        if (ids.empty()) bid_levels.erase(order.price);
    } else {
        auto& ids = ask_levels[order.price];
        ids.erase(std::remove(ids.begin(), ids.end(), order.order_id), ids.end());
        if (ids.empty()) ask_levels.erase(order.price);
    }
}

void OrderBook::apply_delete(const DeleteOrderMessage& msg) {
    auto it = orders_by_id.find(msg.order_id);
    if (it == orders_by_id.end()) return; // unknown order, ignore

    remove_order_from_level(it->second);
    orders_by_id.erase(it);
}

void OrderBook::apply_execute(const OrderExecutedMessage& msg) {
    auto it = orders_by_id.find(msg.order_id);
    if (it == orders_by_id.end()) return;

    RestingOrder& order = it->second;
    if (msg.executed_quantity >= order.quantity) {
        // fully filled -> remove entirely
        remove_order_from_level(order);
        orders_by_id.erase(it);
    } else {
        // partially filled -> just shrink remaining quantity
        order.quantity -= msg.executed_quantity;
    }
}

std::optional<uint32_t> OrderBook::best_bid() const {
    if (bid_levels.empty()) return std::nullopt;
    return bid_levels.begin()->first;
}

std::optional<uint32_t> OrderBook::best_ask() const {
    if (ask_levels.empty()) return std::nullopt;
    return ask_levels.begin()->first;
}

void OrderBook::print_top_of_book() const {
    auto bid = best_bid();
    auto ask = best_ask();

    std::cout << "  Best Bid: " << (bid ? std::to_string(*bid) : "none")
              << " | Best Ask: " << (ask ? std::to_string(*ask) : "none") << "\n";
}

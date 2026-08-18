#include "parser.h"
#include "order_book.h"
#include <vector>
#include <cstring>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <random>

// Builds N synthetic AddOrderMessages directly in memory, skipping file I/O
// entirely -- we want to measure PARSING speed, not disk speed.
std::vector<uint8_t> build_synthetic_feed(size_t num_messages) {
    std::vector<uint8_t> data;
    data.reserve(num_messages * sizeof(AddOrderMessage));

    std::mt19937 rng(42); // fixed seed -> reproducible benchmark runs
    std::uniform_int_distribution<uint32_t> price_dist(1000000, 2000000);
    std::uniform_int_distribution<uint32_t> qty_dist(1, 500);

    uint64_t ts = 0;
    for (size_t i = 0; i < num_messages; ++i) {
        AddOrderMessage msg{};
        msg.message_type = 'A';
        msg.timestamp = ts++;
        msg.order_id = i;
        msg.side = (i % 2 == 0) ? 'B' : 'S';
        msg.price = price_dist(rng);
        msg.quantity = qty_dist(rng);
        std::memset(msg.symbol, ' ', 8);
        std::memcpy(msg.symbol, "AAPL", 4);

        const uint8_t* raw = reinterpret_cast<const uint8_t*>(&msg);
        data.insert(data.end(), raw, raw + sizeof(AddOrderMessage));
    }
    return data;
}

int main() {
    const size_t NUM_MESSAGES = 1'000'000;

    std::cout << "Generating " << NUM_MESSAGES << " synthetic messages in memory...\n";
    auto data = build_synthetic_feed(NUM_MESSAGES);

    // ---------- THROUGHPUT TEST ----------
    // Parse the entire feed once, time the whole thing, then compute
    // messages/sec from total elapsed time.
    OrderBook book;
    auto start = std::chrono::high_resolution_clock::now();

    size_t offset = 0;
    size_t parsed_count = 0;
    while (offset < data.size()) {
        ParsedMessage msg;
        size_t consumed = FeedParser::parse_one(data.data() + offset, data.size() - offset, msg);
        if (consumed == 0) break;

        if (msg.type == MessageType::Add) book.apply_add(msg.add);
        offset += consumed;
        parsed_count++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto total_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    double seconds = total_us / 1'000'000.0;
    double throughput = parsed_count / seconds;

    std::cout << "\n--- Throughput ---\n";
    std::cout << "Parsed " << parsed_count << " messages in " << total_us << " microseconds\n";
    std::cout << "Throughput: " << static_cast<uint64_t>(throughput) << " messages/sec\n";

    // ---------- LATENCY TEST ----------
    // Time each individual parse_one() call separately, in nanoseconds,
    // so we can compute percentiles -- not just an average.
    std::vector<long long> latencies_ns;
    latencies_ns.reserve(NUM_MESSAGES);

    offset = 0;
    while (offset < data.size()) {
        ParsedMessage msg;

        auto t1 = std::chrono::high_resolution_clock::now();
        size_t consumed = FeedParser::parse_one(data.data() + offset, data.size() - offset, msg);
        auto t2 = std::chrono::high_resolution_clock::now();

        if (consumed == 0) break;

        latencies_ns.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
        offset += consumed;
    }

    std::sort(latencies_ns.begin(), latencies_ns.end());

    auto percentile = [&](double p) -> long long {
        size_t idx = static_cast<size_t>(p * latencies_ns.size());
        if (idx >= latencies_ns.size()) idx = latencies_ns.size() - 1;
        return latencies_ns[idx];
    };

    std::cout << "\n--- Latency (per-message parse time) ---\n";
    std::cout << "p50:   " << percentile(0.50) << " ns\n";
    std::cout << "p90:   " << percentile(0.90) << " ns\n";
    std::cout << "p99:   " << percentile(0.99) << " ns\n";
    std::cout << "p99.9: " << percentile(0.999) << " ns\n";
    std::cout << "max:   " << latencies_ns.back() << " ns\n";

    return 0;
}

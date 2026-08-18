# Market Data Feed Handler

![CI](https://github.com/The-Asterix/market-data-feed-handler/actions/workflows/ci.yml/badge.svg)

A low-latency market data feed handler written in C++, simulating a simplified NASDAQ ITCH-style binary protocol — the kind of infrastructure real exchanges and trading firms use to distribute and consume live order data. Built as a hands-on deep dive into low-latency systems design: binary protocols, multithreaded pipelines, and network programming.

## What this project actually does

1. **Generates** synthetic market data (Add/Delete/Execute order events) encoded as compact, fixed-size binary messages
2. **Publishes** that data over a raw TCP socket, simulating a live exchange feed
3. **Subscribes** to the feed on a separate thread from parsing, reassembling messages from a raw byte stream (handling the real-world case where a message can arrive split across two network reads)
4. **Maintains a live in-memory order book**, tracking best bid/ask as events stream in
5. **Benchmarks** raw parsing performance — throughput and latency percentiles (p50/p99/p99.9), not just averages
6. Backed by **unit tests** and a **CI pipeline** that builds and tests the project on every push

## Why binary, not JSON

Real exchange feeds don't use JSON — every byte and every microsecond matters. This project encodes messages as fixed-layout binary structs (`#pragma pack(1)`) and parses them with direct byte offsets, mirroring how real feed handlers are built.

## Architecture

The network thread and parsing thread are deliberately separated, connected by a thread-safe producer-consumer queue — so a slow parse never blocks the next network read, and vice versa. See [`docs/design_notes.md`](docs/design_notes.md) for the full reasoning behind this and other design decisions.

## Roadmap
- [x] Project setup
- [x] Define binary message protocol
- [x] Binary parser
- [x] In-memory order book
- [x] Socket-based publisher
- [x] Multithreaded pipeline
- [x] Latency/throughput benchmarks
- [x] Tests + CI

## Build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Run

Generate sample data:
```bash
./build/generate_data
```

Run the single-process parser + order book:
```bash
./build/feed_handler
```

Run the live publisher/subscriber demo (two terminals):
```bash
# Terminal 1
./build/publisher

# Terminal 2
./build/subscriber
```

Run the benchmark suite:
```bash
./build/benchmark
```
See [`docs/benchmark_results.md`](docs/benchmark_results.md) for recorded throughput and latency numbers.

Run the tests:
```bash
cd build && ctest --output-on-failure
```

## Project structure

## Known limitations

This is a learning project, not production trading infrastructure. See [`docs/design_notes.md`](docs/design_notes.md) for an honest list of simplifications (single symbol, single subscriber, no crash recovery, etc.) and the reasoning behind each design choice.

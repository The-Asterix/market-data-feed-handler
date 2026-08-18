# Market Data Feed Handler

![CI](https://github.com/The-Asterix/market-data-feed-handler/actions/workflows/ci.yml/badge.svg)

A low-latency market data feed handler in C++, simulating a simplified
NASDAQ ITCH-style binary protocol. Built as a learning project to explore
how real trading infrastructure parses and distributes market data.

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
cmake ..
make
./feed_handler
```

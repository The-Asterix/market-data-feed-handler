# Market Data Feed Handler

A low-latency market data feed handler in C++, simulating a simplified
NASDAQ ITCH-style binary protocol. Built as a learning project to explore
how real trading infrastructure parses and distributes market data.

## Roadmap
- [x] Project setup
- [x] Define binary message protocol
- [x] Binary parser
- [ ] In-memory order book
- [ ] Socket-based publisher
- [ ] Multithreaded pipeline
- [ ] Latency/throughput benchmarks
- [ ] Tests + CI

## Build
```bash
mkdir build && cd build
cmake ..
make
./feed_handler
```

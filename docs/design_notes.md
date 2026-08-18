# Design Notes

## Why binary, not JSON/text
Real exchange feeds (like NASDAQ ITCH) use fixed-size binary messages
because parsing is O(1) per field -- no string scanning, no allocations,
no ambiguity. This project mirrors that with #pragma pack(1) structs.

## Why memcpy instead of pointer-casting raw bytes
Casting a uint8_t* directly to a struct pointer risks undefined behavior
on strict-alignment architectures (e.g. ARM, which Apple Silicon uses).
memcpy is safe regardless of alignment and costs effectively nothing for
structs this small.

## Why separate network I/O from parsing (Phase 5)
A single thread doing both read() and parsing means time spent parsing is
time NOT spent listening for new data. Splitting into a producer (network)
and consumer (parser) thread, connected by a thread-safe queue, is the
same pattern used in real low-latency systems to avoid this bottleneck.

## Why the order book is never locked
Only the consumer/parser thread ever touches OrderBook -- the producer
thread only moves raw bytes. This avoids needing a mutex around the book
itself, which would otherwise become a contention point.

## Why percentile latency, not just average
In trading systems, tail latency (p99, p99.9) matters more than the
average case -- a single slow outlier can cost real money. This project's
benchmark reports percentiles instead of a single average number.

## Known simplifications (be upfront about these)
- Real ITCH has many more message types (this project models 3: Add/Delete/Execute)
- No handling of multiple symbols simultaneously in the order book (single-symbol focus)
- No persistence/recovery after a crash
- Publisher sends to one subscriber at a time (real feeds are broadcast to many)

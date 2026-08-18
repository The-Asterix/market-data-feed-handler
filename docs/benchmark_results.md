# Benchmark Results

Measured on: (fill in your machine, e.g. "MacBook Air M2, 2023")
Build type: Release (-O2/-O3 optimizations enabled)
Test size: 1,000,000 synthetic AddOrderMessages

## Throughput
- Messages/sec: <paste your number here>

## Latency (single-message parse time)
- p50:   <paste> ns
- p90:   <paste> ns
- p99:   <paste> ns
- p99.9: <paste> ns
- max:   <paste> ns

## Notes
- Benchmark isolates parsing only (no network I/O, no disk I/O) to
  measure raw CPU-bound performance of the binary parser.
- `memcpy`-based struct population (see `parser.cpp`) avoids
  alignment issues on ARM while staying fast for small, fixed-size structs.
- Latency percentiles are reported instead of just an average, since
  tail latency (p99.9) matters more than typical-case speed in
  low-latency trading systems.

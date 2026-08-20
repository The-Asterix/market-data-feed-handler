# Benchmark Results

Measured on: MacBook Air M2, 2023
Build type: Release (-O2/-O3 optimizations enabled)
Test size: 1,000,000 synthetic AddOrderMessages

## Throughput
- Messages/sec: 1,196,220
- Total time: 835,966 microseconds (~0.84 sec) for 1,000,000 messages

## Latency (single-message parse time)
- p50:   41 ns
- p90:   42 ns
- p99:   42 ns
- p99.9: 83 ns
- max:   18,750 ns

## Notes
- Benchmark isolates parsing only (no network I/O, no disk I/O) to
  measure raw CPU-bound performance of the binary parser.
- `memcpy`-based struct population (see `parser.cpp`) avoids
  alignment issues on ARM while staying fast for small, fixed-size structs.
- Latency percentiles are reported instead of just an average, since
  tail latency (p99.9) matters more than typical-case speed in
  low-latency trading systems.
- The max (18,750 ns) is a clear outlier vs p99.9 (83 ns) — most likely
  caused by OS thread scheduling or a page fault during the run, not the
  parser itself. Worth noting honestly rather than hiding it; this kind
  of tail spike is exactly why p99.9 matters more than max in practice.

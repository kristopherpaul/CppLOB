# C++ Limit Order Book Matching Engine

Two implementations over one semantic contract:
- **Runtime LOB**: Single-threaded, preallocated, hierarchical bitmap
- **Compile-Time LOB**: Type-level state, template metaprogramming, constexpr evaluation

## Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Running Tests

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Benchmarks

### Runtime Benchmarks (Google Benchmark)

```bash
# Configure with benchmarks
cmake -B build -DBUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target runtime_benchmarks

# Run with JSON output
./build/runtime_benchmarks --benchmark_format=json --benchmark_out=results.json

# Run specific benchmark groups
./build/runtime_benchmarks --benchmark_filter="Submit"
./build/runtime_benchmarks --benchmark_filter="Cancel"
./build/runtime_benchmarks --benchmark_filter="BestPrice"
./build/runtime_benchmarks --benchmark_filter="Evolution"
./build/runtime_benchmarks --benchmark_filter="Allocation"
```

**Benchmark Configurations** (CMake options):
- `-DBENCHMARK_CONFIG_SMALL=ON` - 1-1000 prices, 10K orders
- `-DBENCHMARK_CONFIG_MEDIUM=ON` - 1-5000 prices, 50K orders (default)
- `-DBENCHMARK_CONFIG_LARGE=ON` - 1-10000 prices, 100K orders

### Runtime Benchmark Suite

| Benchmark | Metrics | Description |
|-----------|---------|-------------|
| `runtime_submit.cpp` | p50/p90/p99/p99.9 latency, throughput | Submit with various cross ratios (0%, 25%, 50%, 75%, 100%) |
| `runtime_cancel.cpp` | p50/p90/p99/p99.9 latency | Cancel head/middle/tail, miss, after partial fill |
| `runtime_best_price.cpp` | p50/p90/p99/p99.9 latency | Empty, single level, sparse/dense bitmap, after each op |
| `runtime_evolution.cpp` | Latency per stage, speedup | 7 stages: Reference → Flat ladder → Intrusive FIFO → Fixed Pool → Order Index → Hierarchical Bitmap → Side-Specialized |
| `runtime_allocation.cpp` | Allocation count (target: 0) | Hot-path allocation tracking after initialization |

### Compile-Time Benchmarks

```bash
# Generate compile-time benchmark configurations
cmake --build build --target compile_time_benchmarks_generate

# Re-configure to create targets from generated sources
cmake -B build -DBUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target ct_compile_time_scaling_ops10_pls10

# Or build all compile-time benchmarks
cmake --build build --target ct_compile_time_scaling_ops1_pls1 ct_compile_time_scaling_ops2_pls1 ...
```

**Generated Configurations** (25 combinations):
- Orders per side: 1, 2, 5, 10, 20, 50, 100
- Price levels per side: 1, 2, 5, 10, 20

### Compile-Time Benchmark Metrics

| Metric | Method |
|--------|--------|
| Compilation time | `scripts/compile_timer.py` wrapper (CXX_COMPILER_LAUNCHER) |
| Compiler memory | GNU `time -v` parsing (Max RSS) |
| Binary size | `size` command on executables |

### Binary Size Measurement

```bash
cmake --build build --target measure_binary_size
```

### Data-Structure Evolution Benchmark (7 Stages)

Measuring incremental optimization impact:

| Stage | Implementation | Key Data Structure |
|-------|---------------|-------------------|
| 0 | Reference | `std::map<Price, std::deque<Order>>` |
| 1 | Flat Ladder | `std::array<PriceLevel, N>` + linear scan |
| 2 | Intrusive FIFO | 32-bit indices, prev/next pointers |
| 3 | Fixed Order Pool | `OrderPool<MaxOrders>` with free list |
| 4 | Order-ID Index | Open-addressing hash table (O(1) cancel) |
| 5 | Hierarchical Bitmap | L0/L1 bitmap for O(1) best price |
| 6 | Side-Specialized | `if constexpr (Side)` template dispatch |

### Results (Intel Core i5, GCC 11.4, C++23)

```
# Submit Latency (ns) - 10K orders, no-cross
BM_Submit_NoCross/10000        45 ns/op (p50)  67 ns (p90)  89 ns (p99)  112 ns (p99.9)

# Cancel Latency (ns) - 1000 orders at same price
BM_Cancel_Head/1000            12 ns/op (p50)  18 ns (p90)  24 ns (p99)

# Best Price Query (ns) - Dense bitmap (10K levels)
BM_BestPrice_Dense             3 ns/op (p50)   5 ns (p90)   8 ns (p99)

# Evolution Speedup vs Reference
Stage 1 (Flat Ladder):         12.5x faster
Stage 2 (Intrusive FIFO):      18.2x faster
Stage 3 (Fixed Pool):          22.1x faster
Stage 4 (Order Index):         22.3x faster (cancel: 500x faster)
Stage 5 (Hierarchical Bitmap): 22.5x faster (best_price: 2000x faster)
Stage 6 (Side Specialized):    24.8x faster

# Allocation Count (Hot Path)
Submit:     0 allocs
Cancel:     0 allocs
Match:      0 allocs
Best Price: 0 allocs

# Compile-Time Scaling (GCC 11.4)
Orders  Levels  Compile Time  Memory
1       1       ~0.5s         ~38 MB
10      10      ~0.6s         ~54 MB
50      10      ~1.2s         ~180 MB
100     20      ~1.5s         ~335 MB (hits template depth limits)
```

### Methodology

- **Pre-generated workloads**: All orders generated outside timed regions
- **Warmup**: Google Benchmark automatic warmup iterations
- **Statistics**: p50, p90, p99, p99.9 percentiles via Google Benchmark
- **Allocation tracking**: Custom `operator new`/`delete` override in `runtime_allocation.cpp`
- **Compile-time measurement**: `scripts/compile_timer.py` wraps compiler with `time -v`
- **CPU pinning**: Recommended `cpupower frequency-set -g performance` for consistent results

### Files

```
benchmarks/
├── runtime/
│   ├── runtime_submit.cpp       # Submit latency benchmarks
│   ├── runtime_cancel.cpp       # Cancel latency benchmarks
│   ├── runtime_best_price.cpp   # Best price query benchmarks
│   ├── runtime_evolution.cpp    # 7-stage evolution benchmarks
│   ├── runtime_allocation.cpp   # Allocation counting
│   ├── workloads.hpp/cpp        # Workload generators
│   └── evolution/               # Stage implementations
│       ├── stage0_reference.hpp
│       ├── stage1_flat_ladder.hpp
│       ├── stage2_intrusive_fifo.hpp
│       ├── stage3_fixed_pool.hpp
│       ├── stage4_order_index.hpp
│       ├── stage5_hierarchical_bitmap.hpp
│       └── stage6_side_specialized.hpp
├── compile_time/
│   ├── compile_time_scaling.cpp.in    # Template for scaling tests
│   └── generated/                     # Auto-generated configurations
scripts/
├── compile_timer.py             # Compiler timing wrapper
└── generate_ct_benchmarks.py    # Configuration generator
```
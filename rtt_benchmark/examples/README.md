# RTT Benchmark Examples

This directory contains examples demonstrating the use of the RTT benchmarking library.

## benchmark_example.cpp

A comprehensive example showing different ways to use the benchmarking functionality:

1. **Automatic Reporting**: Run a benchmark and automatically report results via RTT
2. **Manual Stats Handling**: Run a benchmark, inspect statistics, then report
3. **Multiple Iterations Comparison**: Compare performance with different iteration counts
4. **Scoped Timer**: Simple timing of code blocks using RAII

## Building the Examples

Examples are built when `BUILD_EXAMPLES` is enabled (default: ON).

### Native Build

```bash
cmake --preset default
cmake --build --preset default
./build/default/rtt_benchmark/benchmark_example
```

### ARM Build

```bash
cmake --preset arm-stm32f205
cmake --build --preset arm-stm32f205
# Flash to target device and view output via J-Link RTT Viewer
```

## Expected Output

When run on a target with RTT output viewer, you should see:

```
===========================================
  RTT Benchmark Example
===========================================

Example 1: Quick operation benchmark
-------------------------------------------
Starting benchmark...
=== Benchmark Results ===
Name: QuickOperation
Iterations: 100
Min time: 2 us
Max time: 8 us
Mean time: 3 us
Total time: 345 us
========================

Example 2: Medium operation benchmark
-------------------------------------------
=== Benchmark Results ===
Name: MediumOperation
Iterations: 50
Min time: 15 us
Max time: 24 us
Mean time: 18 us
Total time: 912 us
========================

...
```

## Using the Benchmark Library

Include the header:
```cpp
#include <rtt_benchmark/rtt_benchmark.hpp>
```

### Basic Benchmarking

```cpp
rtt::benchmark::Benchmark bench("MyFunction");

// Run and get statistics
auto stats = bench.run([]() {
    myFunction();
}, 100);  // 100 iterations

// Or run and automatically report
bench.runAndReport([]() {
    myFunction();
}, 100);
```

### Scoped Timing

```cpp
{
    rtt::benchmark::ScopedTimer timer("MyOperation");
    doSomething();
}  // Time automatically reported when scope ends
```

## Performance Considerations

- Benchmarking adds overhead due to timing measurements
- Use sufficient iterations for stable results (typically 10-1000)
- First iteration may be slower due to cache effects
- Results are reported in microseconds (μs)
- Maximum 10,000 iterations per benchmark to avoid memory issues

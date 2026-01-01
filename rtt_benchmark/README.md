# RTT Benchmark

Code benchmarking and performance measurement library using SEGGER RTT for embedded systems.

## Features

- **Automated benchmarking** - Run functions multiple times and collect statistics
- **Statistical analysis** - Min, max, mean, and total execution times
- **Scoped timing** - RAII-based timing for code blocks
- **RTT reporting** - Real-time performance data via RTT
- **Flexible iteration counts** - Configurable repetitions for stable results
- **Template-based** - Works with any callable (functions, lambdas, functors)
- **C++20 concepts** - Compile-time validation of benchmarkable functions (when available)
- **Minimal overhead** - Optimized timing measurement

## Requirements

- C++20 for concepts
- RTT Logger library
- SEGGER RTT library
- CMake 3.20 or later
- High-resolution timer support (uses `std::chrono`)

## Integration

Add to your CMakeLists.txt:

```cmake
target_link_libraries(your_application
    PRIVATE
        rtt_benchmark
        rtt_logger
)
```

## Quick Start

```cpp
#include <rtt_benchmark/rtt_benchmark.hpp>
#include <rtt_logger/rtt_logger.hpp>

int main() {
    // Initialize RTT
    rtt::Logger::initialize();
    
    // Create benchmark
    rtt::benchmark::Benchmark bench("MyFunction");
    
    // Run and automatically report results
    bench.runAndReport<100>([]() {
        // Code to benchmark
        for (int i = 0; i < 1000; ++i) {
            volatile int x = i * i;
        }
    });  // Run 100 iterations
    
    return 0;
}
```

## API Reference

### Benchmark Class

```cpp
class Benchmark {
public:
    // Constructor
    explicit Benchmark(std::string_view name, Logger& logger = rtt::getLogger());
    
    // Run benchmark and get statistics
    template<size_t Iterations, typename Func>
    BenchmarkStats run(Func&& func);
    
    // Run benchmark and automatically report via RTT
    template<size_t Iterations, typename Func>
    void runAndReport(Func&& func);
    
    // Report statistics via RTT
    void report(const BenchmarkStats& stats);
};
```

### BenchmarkStats Structure

```cpp
struct BenchmarkStats {
    uint64_t min;         // Minimum execution time (µs)
    uint64_t max;         // Maximum execution time (µs)
    uint64_t mean;        // Mean execution time (µs)
    uint64_t total;       // Total execution time (µs)
    size_t iterations;    // Number of iterations performed
};
```

### ScopedTimer Class

```cpp
class ScopedTimer {
public:
    // Constructor - starts timing
    explicit ScopedTimer(std::string_view name, Logger& logger = rtt::getLogger());
    
    // Destructor - stops timing and reports via RTT
    ~ScopedTimer();
};
```

## Usage Examples

### Basic Benchmarking

```cpp
rtt::benchmark::Benchmark bench("QuickSort");

auto stats = bench.run<50>([]() {
    quickSort(data, size);
});  // Run 50 times

// Use statistics
if (stats.mean > 1000) {
    logger.warning("Sort is slower than expected!");
}

bench.report(stats);
```

### Automatic Reporting

```cpp
rtt::benchmark::Benchmark bench("MatrixMultiply");

bench.runAndReport<100>([]() {
    multiplyMatrices(A, B, result);
});
```

### Scoped Timing

```cpp
void processData() {
    {
        rtt::benchmark::ScopedTimer timer("DataProcessing");
        
        // All code in this scope is timed
        parseInput();
        transform();
        writeOutput();
        
    }  // Timer automatically reports when scope exits
}
```

### Comparing Implementations

```cpp
// Benchmark implementation A
rtt::benchmark::Benchmark benchA("Algorithm_A");
auto statsA = benchA.run<100>(algorithmA);

// Benchmark implementation B
rtt::benchmark::Benchmark benchB("Algorithm_B");
auto statsB = benchB.run<100>(algorithmB);

// Compare results
benchA.report(statsA);
benchB.report(statsB);

if (statsA.mean < statsB.mean) {
    logger.info("Algorithm A is faster");
} else {
    logger.info("Algorithm B is faster");
}
```

### Benchmarking with Parameters

```cpp
// Benchmark with different input sizes
for (size_t size : {100, 1000, 10000}) {
    std::string name = "Sort_" + std::to_string(size);
    rtt::benchmark::Benchmark bench(name);
    
    bench.runAndReport<50>([&]() {
        sort(data, size);
    });
}
```

## Example Output

```
=== Benchmark Results ===
Name: QuickSort
Iterations: 100
Min time: 245 us
Max time: 312 us
Mean time: 267 us
Total time: 26700 us
========================
```

## Examples

See the [examples](examples/) directory for complete examples:

- `benchmark_example.cpp` - Comprehensive demonstration of benchmarking features

### Building Examples

```bash
# Native build
cmake --preset default
cmake --build --preset default
./build/default/rtt_benchmark/benchmark_example

# ARM build
cmake --preset arm-stm32f205
cmake --build --preset arm-stm32f205
# Flash and view output via RTT
```

## Best Practices

1. **Sufficient iterations**: Use 10-1000 iterations for stable results
2. **Warm-up**: First iteration may be slower due to cache effects
3. **Consistent conditions**: Benchmark under similar system load
4. **Realistic scenarios**: Use representative input data
5. **Avoid optimization tricks**: Ensure benchmarked code isn't optimized away
   ```cpp
   // Use volatile to prevent optimization
   volatile int result = calculate();
   ```
6. **Measure what matters**: Focus on actual performance bottlenecks
7. **Compare fairly**: Use same iteration count when comparing

## C++20 Features

When using C++20, the library uses concepts for type safety:

```cpp
template<typename F>
concept BenchmarkableFunction = std::invocable<F> && 
                                std::same_as<void, std::invoke_result_t<F>>;
```

This ensures at compile-time that:
- The function can be called with no arguments
- The function returns void (or return value is ignored)

## Performance Considerations

### Timing Overhead
- Benchmark overhead: ~1-2µs per iteration (typical)
- ScopedTimer overhead: ~0.5-1µs total
- Use sufficient iterations to minimize relative overhead

### Memory Usage
- Benchmark class: ~50 bytes
- BenchmarkStats: ~40 bytes
- ScopedTimer: ~30 bytes

### Accuracy
- Resolution: Microsecond (µs) precision
- Platform-dependent: Uses `std::chrono::high_resolution_clock`
- For sub-microsecond timing, consider platform-specific timers

## Troubleshooting

### Inconsistent results
1. Run more iterations for better averaging
2. Disable interrupts during benchmarking (if possible)
3. Ensure consistent system state between runs
4. Check for background tasks affecting timing

### Results seem wrong
1. Verify code isn't optimized away (use volatile)
2. Check iteration count is sufficient
3. Ensure RTT output isn't affecting timing
4. Consider measurement overhead for very fast operations

### No output via RTT
1. Ensure `Logger::initialize()` is called
2. Check RTT viewer is connected to channel 0
3. Verify debug probe connection

## Advanced Usage

### Custom Logger

```cpp
rtt::Logger customLogger;
customLogger.setMinLevel(rtt::LogLevel::Debug);

rtt::benchmark::Benchmark bench("Custom", customLogger);
```

### Integration with Testing

```cpp
TEST(PerformanceTest, SortSpeed) {
    rtt::benchmark::Benchmark bench("SortTest");
    auto stats = bench.run<100>(sortFunction);
    
    // Assert performance requirements
    EXPECT_LT(stats.mean, 500);  // Must be under 500µs
}
```

## Reading RTT Output

### Using J-Link
```bash
python3 scripts/rtt_reader.py --backend jlink --device STM32F205RB
```

### Using OpenOCD
```bash
# Start OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg

# Read benchmark output
python3 scripts/rtt_reader.py --backend openocd --host localhost --port 4444
```

## See Also

- [Main README](../README.md) - Project overview
- [RTT Logger](../rtt_logger/) - Core logging functionality
- [Examples](examples/README.md) - Detailed example documentation

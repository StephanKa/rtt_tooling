# RTT Tools

[![ARM GCC Build and Test](https://github.com/StephanKa/rtt_tooling/actions/workflows/build.yml/badge.svg)](https://github.com/StephanKa/rtt_tooling/actions/workflows/build.yml)

A comprehensive C++ toolkit for SEGGER Real-Time Transfer (RTT) with support for embedded logging, unit testing, FreeRTOS integration, code benchmarking, and memory dumping.

## Features

- **Modern C++17/20/23** RTT logger with type-safe interfaces
  - C++20 concepts for type constraints
  - C++20 std::span for safer array handling
  - C++20 ranges support
  - Backward compatible with C++17
- **Modern C++17** RTT logger with type-safe interfaces
- **External SEGGER RTT library** fetched automatically via CMake FetchContent
- **CMake & Ninja** build system with presets
- **Configurable C++ standard** (17, 20, 23) via CMake option
- **ARM GCC toolchain** support for STM32F205
- **GoogleTest integration** for unit testing with RTT output support
- **Host-side RTT reading** via OpenOCD and SEGGER J-Link
- **Optimized GoogleTest** builds for embedded systems (size-optimized)
- **FreeRTOS hooks** for system diagnostics via RTT
- **FreeRTOS tracing** for task, interrupt, and timing analysis via RTT
- **Benchmarking tools** for measuring code execution performance
- **Memory dump utilities** for dumping memory regions via RTT with multiple formats
- **Generic data transmission** for sending structured data via RTT
- **Fault handler** for ARM Cortex-M with stack traces and comprehensive error reporting via RTT
- **Python scripts** for RTT log viewing and analysis
- **Dual probe support** for both J-Link and OpenOCD (ST-Link)

## Project Structure

```
rtt_tooling/
├── rtt_logger/              # RTT logger library (modern C++17/20/23)
│   ├── include/
│   │   └── rtt_logger/
│   │       └── rtt_logger.hpp      # Modern C++ RTT logger interface
│   └── src/
│       └── rtt_logger.cpp
│
├── rtt_unittest/            # RTT unit testing with GoogleTest
│   ├── include/
│   │   └── rtt_unittest/
│   │       └── rtt_unittest.hpp    # Unit test helpers
│   ├── src/
│   │   └── rtt_unittest.cpp
│   └── tests/
│       ├── test_rtt_logger.cpp
│       ├── test_rtt_unittest.cpp
│       └── test_main_rtt.cpp   # Custom main for RTT output
│
├── rtt_freertos_hooks/      # FreeRTOS integration
│   ├── include/
│   │   └── rtt_freertos_hooks/
│   │       └── rtt_freertos_hooks.hpp
│   └── src/
│       └── rtt_freertos_hooks.cpp
│
├── rtt_freertos_trace/      # FreeRTOS tracing via RTT
│   ├── include/
│   │   └── rtt_freertos_trace/
│   │       └── rtt_freertos_trace.hpp
│   ├── src/
│   │   └── rtt_freertos_trace.cpp
│   ├── examples/
│   │   └── example_trace.cpp
│   └── README.md            # Detailed tracing documentation
│
├── rtt_benchmark/           # Code benchmarking and performance testing
│   ├── include/
│   │   └── rtt_benchmark/
│   │       └── rtt_benchmark.hpp   # Benchmark utilities
│   └── src/
│       └── rtt_benchmark.cpp
│
├── rtt_memory_dump/         # Memory dump utilities via RTT
│   ├── include/
│   │   └── rtt_memory_dump/
│   │       └── rtt_memory_dump.hpp # Memory dump utilities
│   ├── src/
│   │   └── rtt_memory_dump.cpp
│   └── examples/
│       └── memory_dump_example.cpp
├── rtt_data/                # Generic data transmission via RTT
│   ├── include/
│   │   └── rtt_data/
│   │       └── rtt_data.hpp        # Data sending interface
│   ├── src/
│   │   └── rtt_data.cpp
│   └── examples/
│       └── data_example.cpp
├── rtt_fault_handler/       # Hardware fault handling with stack traces
│   ├── include/
│   │   └── rtt_fault_handler/
│   │       └── rtt_fault_handler.hpp  # Fault handler interface
│   ├── src/
│   │   └── rtt_fault_handler.cpp
│   └── examples/
│       └── fault_handler_example.cpp
│
├── scripts/                 # Python utilities
│   ├── rtt_reader.py       # RTT reader for OpenOCD and J-Link
│   ├── rtt_viewer.py       # RTT output viewer
│   ├── rtt_analyzer.py      # Log analysis tool
│   ├── rtt_trace_reader.py  # FreeRTOS trace reader (J-Link/OpenOCD)
│   ├── rtt_trace_analyzer.py # Trace data analyzer
│   └── rtt_data_reader.py   # Structured data reader and formatter
│
├── docs/                    # Documentation
│   └── googletest_size_optimization.md  # GoogleTest size reduction guide
│
├── cmake/                   # CMake toolchain files
│   └── arm-none-eabi-gcc.cmake
│
├── CMakeLists.txt
├── CMakePresets.json
└── .github/
    └── workflows/
        └── build.yml       # CI/CD for ARM GCC builds
```

## Requirements

### For Native Builds
- CMake 3.20 or later
- Ninja build system
- C++17/20/23 compatible compiler (GCC 10+, Clang 11+)
- Internet connection (for fetching SEGGER RTT library on first build)

### For ARM Builds
- ARM GCC toolchain (arm-none-eabi-gcc)
- CMake 3.20 or later
- Ninja build system
- Internet connection (for fetching SEGGER RTT library on first build)

### For Testing
- GoogleTest (automatically fetched by CMake)
- SEGGER RTT library (automatically fetched by CMake from https://github.com/SEGGERMicro/RTT)

### For Python Scripts
- Python 3.6 or later
- Optional: pylink-square (for J-Link RTT support)
  ```bash
  pip install pylink-square
  ```

## Building

### C++ Standard Selection

The project supports C++17, C++20, and C++23. The default is C++20.

To select a different C++ standard:

```bash
# Build with C++17
cmake --preset default -DRTT_CXX_STANDARD=17
cmake --build --preset default

# Build with C++20 (default)
cmake --preset default
cmake --build --preset default

# Build with C++23
cmake --preset default -DRTT_CXX_STANDARD=23
cmake --build --preset default
```

**Modern C++ Features by Standard:**

- **C++17**: Core functionality with `string_view`, `constexpr`, and basic type traits
- **C++20** (recommended): 
  - Concepts for type constraints (`Formattable`, `BenchmarkableFunction`)
  - `std::span` for safer array handling in benchmarks
  - `std::ranges` for container algorithms
- **C++23**: All C++20 features plus latest standard features

### Using CMake Presets

The project uses CMake presets for different build configurations:

```bash
# Native debug build
cmake --preset default
cmake --build --preset default

# Native release build
cmake --preset release
cmake --build --preset release

# ARM STM32F205 debug build
cmake --preset arm-stm32f205
cmake --build --preset arm-stm32f205

# ARM STM32F205 release build
cmake --preset arm-stm32f205-release
cmake --build --preset arm-stm32f205-release

# Build with tests enabled
cmake --preset testing
cmake --build --preset testing
ctest --preset testing
```

### Manual Configuration

```bash
# Native build with custom C++ standard
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DRTT_CXX_STANDARD=20 ..
ninja

# ARM build for STM32F205
mkdir build-arm && cd build-arm
cmake -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-none-eabi-gcc.cmake \
  -DTARGET_DEVICE=STM32F205 \
  -DCMAKE_BUILD_TYPE=Debug \
  -DRTT_CXX_STANDARD=20 \
  ..
ninja
```

## Usage

### RTT Logger (C++)

```cpp
#include <rtt_logger/rtt_logger.hpp>

int main() {
    // Initialize RTT
    rtt::Logger::initialize();
    
    // Get global logger instance
    auto& logger = rtt::getLogger();
    logger.setMinLevel(rtt::LogLevel::Debug);
    
    // Log messages
    logger.info("Application started");
    logger.debug("Debug information");
    logger.warning("Warning message");
    logger.error("Error occurred");
    
    // Formatted logging
    logger.logFormatted(rtt::LogLevel::Info, "Value: %d", 42);
    
    return 0;
}
```

### Unit Testing

#### Standard Unit Tests (Console Output)
```cpp
#include <gtest/gtest.h>
#include <rtt_logger/rtt_logger.hpp>

TEST(RttLoggerTest, BasicLogging) {
    auto& logger = rtt::getLogger();
    logger.setMinLevel(rtt::LogLevel::Trace);
    
    EXPECT_EQ(logger.getMinLevel(), rtt::LogLevel::Trace);
    EXPECT_TRUE(logger.isEnabled(rtt::LogLevel::Info));
}
```

Run tests:
```bash
cmake --preset testing
cmake --build --preset testing
cd build/testing
ctest --output-on-failure
```

#### Unit Tests with RTT Output (Embedded)

For embedded targets, use the RTT-enabled test executable that outputs all GoogleTest results via RTT:

```cpp
// Test main with RTT output is automatically included
// Just build rtt_unittest_tests_rtt target
```

Build and flash to embedded target:
```bash
cmake --preset arm-stm32f205
cmake --build --preset arm-stm32f205 --target rtt_unittest_tests_rtt
# Flash the rtt_unittest_tests_rtt binary to your device
```

Read test results via RTT:
```bash
# Using OpenOCD
python3 scripts/rtt_reader.py --backend openocd --host localhost --port 4444

# Using J-Link
python3 scripts/rtt_reader.py --backend jlink --device STM32F205RB
```

### FreeRTOS Hooks

```cpp
#include <rtt_freertos_hooks/rtt_freertos_hooks.hpp>

// Enable verbose hook logging
rtt::freertos::FreeRtosHooks::setVerbose(true);

// The hooks are automatically called by FreeRTOS:
// - vApplicationMallocFailedHook()
// - vApplicationStackOverflowHook()
// - vApplicationTickHook()
// - vApplicationIdleHook()
// - vApplicationDaemonTaskStartupHook()
```

### FreeRTOS Tracing

Comprehensive tracing for FreeRTOS without commercial tools. Captures task switches, interrupts, and timing information.

```cpp
#include <rtt_freertos_trace/rtt_freertos_trace.hpp>

// Initialize trace system (channel 1)
rtt_trace_init(1);

// Register tasks for readable output
rtt_trace_register_task((uint32_t)task_handle, "MyTask", 6);

// Start tracing
rtt_trace_start();
```

#### Capture Traces

```bash
# Using J-Link
python3 scripts/rtt_trace_reader.py -d STM32F205RB -p jlink -c 1 -o trace.bin

# Using OpenOCD with ST-Link
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg &
python3 scripts/rtt_trace_reader.py -d stm32f2x -p openocd -c 1 -o trace.bin
```

#### Analyze Traces

```bash
# Task runtime analysis
python3 scripts/rtt_trace_analyzer.py trace.bin --task-runtime

# Interrupt analysis
python3 scripts/rtt_trace_analyzer.py trace.bin --interrupts

# Event timeline
python3 scripts/rtt_trace_analyzer.py trace.bin --timeline

# Export to JSON
python3 scripts/rtt_trace_analyzer.py trace.bin --export-json trace.json

# Export to Chrome Trace format (view in chrome://tracing)
python3 scripts/rtt_trace_analyzer.py trace.bin --export-chrome-trace trace.json

# Export to Perfetto format (view in https://ui.perfetto.dev/)
python3 scripts/rtt_trace_analyzer.py trace.bin --export-perfetto trace.json
```

**New Features:**
- **Chrome Trace Export**: Visualize traces in Chrome's built-in trace viewer (`chrome://tracing`)
- **Perfetto Export**: Advanced trace visualization in Perfetto UI (https://ui.perfetto.dev/)
- **Memory Hooks**: Automatic tracking of FreeRTOS memory allocations (malloc/free)

**See [rtt_freertos_trace/README.md](rtt_freertos_trace/README.md) for detailed documentation.**

### Code Benchmarking

```cpp
#include <rtt_benchmark/rtt_benchmark.hpp>

// Run a function multiple times and get statistics
rtt::benchmark::Benchmark bench("MyFunction");
auto stats = bench.run([]() {
    // Code to benchmark
    for (int i = 0; i < 1000; ++i) {
        volatile int x = i * i;
    }
}, 100);  // Run 100 iterations

// Or run and automatically report via RTT
bench.runAndReport([]() {
    // Code to benchmark
    performComplexCalculation();
}, 50);

// For simple scope timing
{
    rtt::benchmark::ScopedTimer timer("Operation");
    // Code to measure
    doSomething();
}  // Elapsed time automatically reported when scope ends
```

Example output via RTT:
```
=== Benchmark Results ===
Name: MyFunction
Iterations: 100
Min time: 245 us
Max time: 312 us
Mean time: 267 us
Total time: 26700 us
```

### Memory Dump

Dump memory regions via RTT for debugging and analysis. Supports multiple output formats.

```cpp
#include <rtt_memory_dump/rtt_memory_dump.hpp>

// Create a memory dumper
rtt::memory_dump::MemoryDumper dumper;

// Dump a structure with default hex+ASCII format
struct SensorData {
    uint32_t timestamp;
    float temperature;
    float pressure;
};

SensorData sensor = {0x12345678, 23.5f, 1013.25f};
dumper.dumpObject(sensor, "Sensor Data");

// Dump memory with hex-only format
rtt::memory_dump::DumpConfig cfg(rtt::memory_dump::DumpFormat::Hex);
dumper.setConfig(cfg);
dumper.dump(buffer, buffer_size, "Buffer Contents");

// Available formats:
// - DumpFormat::Hex        (hexadecimal only)
// - DumpFormat::HexAscii   (hex + ASCII representation)
// - DumpFormat::Binary     (binary 0s and 1s)
// - DumpFormat::Decimal    (decimal values)

// Custom configuration
rtt::memory_dump::DumpConfig custom_cfg;
custom_cfg.bytes_per_line = 8;      // 8 bytes per line
custom_cfg.format = rtt::memory_dump::DumpFormat::HexAscii;
custom_cfg.show_address = true;      // Show memory addresses

rtt::memory_dump::MemoryDumper custom_dumper(custom_cfg);
custom_dumper.dump(data, size, "Custom Format");

#if __cplusplus >= 202002L
// C++20 span-based dump
std::span<const uint8_t> data_span(buffer);
dumper.dump(data_span, "Span Data");
#endif
```

Example output via RTT:
```
=== Memory Dump: Sensor Data ===
Address: 0x20001000, Size: 12 bytes
0x20001000: 78 56 34 12 00 00 BC 41 00 00 7E 44 | xV4....A..~D
=== End Memory Dump ===
### Generic Data Transmission

The rtt_data library provides a type-safe interface for sending structured data via RTT to the host, with automatic type identification and optional timestamping.

```cpp
#include <rtt_data/rtt_data.hpp>
#include <rtt_logger/rtt_logger.hpp>

int main() {
    // Initialize RTT
    rtt::Logger::initialize();
    
    // Get the global data sender (uses RTT channel 1 by default)
    auto& dataSender = rtt::data::getDataSender();
    
    // Send integers of different sizes
    dataSender.sendInt(static_cast<int32_t>(42));
    dataSender.sendInt(static_cast<uint16_t>(1000));
    
    // Send floating-point values
    dataSender.sendFloat(3.14159f);
    dataSender.sendFloat(2.71828);
    
    // Send strings
    dataSender.sendString("Hello from RTT!");
    
    // Send binary data
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    dataSender.sendBinary(data, sizeof(data));
    
    // Enable timestamping
    dataSender.setTimestamping(true);
    for (int i = 0; i < 10; ++i) {
        dataSender.sendInt(static_cast<int32_t>(i * 100));
    }
    
    // Send custom struct (must be trivially copyable)
    struct SensorData {
        float temperature;
        float humidity;
        uint32_t pressure;
    } __attribute__((packed));
    
    SensorData sensor = {23.5f, 65.2f, 101325};
    dataSender.send(sensor);
### Fault Handler (ARM Cortex-M)

The fault handler provides comprehensive error and exception handling for ARM Cortex-M processors with detailed stack traces sent via RTT.

```cpp
#include <rtt_fault_handler/rtt_fault_handler.hpp>

int main() {
    // Initialize the fault handler with default configuration
    // This overrides HardFault, MemManage, BusFault, and UsageFault handlers
    rtt::fault::FaultHandler::initialize();
    
    // Alternative: Initialize with custom configuration
    rtt::fault::FaultHandlerConfig config;
    config.rttChannel = 0;           // Use RTT channel 0
    config.maxStackDepth = 32;       // Show up to 32 stack frames
    config.enableVerbose = true;     // Enable verbose fault decoding
    rtt::fault::FaultHandler::initialize(config);
    
    // Your application code here
    // Any hardware fault will be caught and reported via RTT
    
    return 0;
}
```

#### Reading Data on Host

Use the Python data reader to receive and format data from the embedded device:

```bash
# Read from device via OpenOCD
python3 scripts/rtt_data_reader.py --backend openocd --host localhost --port 4444 --channel 1

# Read from device via J-Link
python3 scripts/rtt_data_reader.py --backend jlink --device STM32F205RB --channel 1

# Parse binary file
python3 scripts/rtt_data_reader.py --file data.bin

# Verbose output for debugging
python3 scripts/rtt_data_reader.py --backend openocd --verbose
```

Example output:
```
Reading RTT data from channel 1...
Press Ctrl+C to stop

[Int32] 42
[UInt16] 1000
[Float] 3.141590
[Double] 2.718280
[String] "Hello from RTT!"
[Binary] 0xdeadbeef
[000001] [Int32] 0
[000002] [Int32] 100
[000003] [Int32] 200
[Binary] 0x0000bc41cdcc84425d900100
**Features:**
- Overrides all ARM Cortex-M hardware fault handlers:
  - HardFault
  - MemManage Fault
  - BusFault
  - UsageFault
- Provides detailed fault information via RTT:
  - Fault type identification
  - CPU register dump (R0-R3, R12, LR, PC, PSR)
  - Fault status registers (CFSR, HFSR, DFSR, MMFAR, BFAR)
  - Decoded fault cause (verbose mode)
  - Stack trace with configurable depth
- Automatic stack pointer detection (MSP vs PSP)
- Cross-platform compatible (stubs for non-ARM builds)

**Example fault output via RTT:**
```
     FAULT EXCEPTION DETECTED    
Fault Type: HardFault

--- CPU Registers ---
R0  = 0x00000000
R1  = 0x20001234
R2  = 0x00000042
R3  = 0x00000000
R12 = 0x00000000
LR  = 0x080012AB
PC  = 0x08001234
PSR = 0x21000000

--- Fault Status Registers ---
CFSR  = 0x00000200
HFSR  = 0x40000000
DFSR  = 0x00000000
MMFAR = 0x00000000
BFAR  = 0x00000000

--- CFSR Decode ---
  PRECISERR: Precise data bus error

--- Stack Trace ---
Stack Pointer: 0x20000F80

Stack dump (first frames):
  [00] 0x20000F80: 0x00000000
  [01] 0x20000F84: 0x20001234
  [02] 0x20000F88: 0x080012AB
  ...
```

### Python Scripts

#### RTT Reader (OpenOCD and J-Link Support)

The RTT reader provides host-side reading of RTT output from embedded devices, with support for both OpenOCD and SEGGER J-Link.

```bash
# Read from device via OpenOCD
python3 scripts/rtt_reader.py --backend openocd --host localhost --port 4444

# Read from device via J-Link
python3 scripts/rtt_reader.py --backend jlink --device STM32F205RB --interface SWD

# Save output to file
python3 scripts/rtt_reader.py --backend jlink --device STM32F205RB --output test_results.txt

# Read from specific RTT channel
python3 scripts/rtt_reader.py --backend openocd --channel 1
```

**OpenOCD Setup:**
1. Start OpenOCD with your target configuration:
   ```bash
   openocd -f interface/stlink.cfg -f target/stm32f2x.cfg
   ```
2. Run the RTT reader to connect to OpenOCD's telnet interface (default port 4444)

**J-Link Setup:**
1. Install pylink: `pip install pylink-square`
2. Connect J-Link to your target
3. Run the RTT reader with appropriate device name

#### RTT Viewer (Legacy)
```bash
# View RTT output from STM32F205
python3 scripts/rtt_viewer.py -d STM32F205RB -i SWD -c 0

# View with custom settings
python3 scripts/rtt_viewer.py --device STM32F405RG --speed 8000
```

#### RTT Log Analyzer
```bash
# Analyze log file
python3 scripts/rtt_analyzer.py logfile.txt

# Filter by log level
python3 scripts/rtt_analyzer.py logfile.txt --level ERROR

# Search for pattern
python3 scripts/rtt_analyzer.py logfile.txt --search "timeout" -i

# Show only statistics
python3 scripts/rtt_analyzer.py logfile.txt --stats-only
```

## Binary Statistics

The project uses ARM GCC compiler for embedded targets. Binary statistics are automatically generated for each build to track code size.

### ARM Debug Build

The debug build includes debugging symbols and minimal optimizations for better debugging experience:

| Component | text (ROM) | data (RAM) | bss (RAM) | Total |
|-----------|------------|------------|-----------|-------|
| **Total Libraries** | Generated by CI | Generated by CI | Generated by CI | Generated by CI |

**Section Breakdown:**
- `.text` - Code in ROM
- `.data` - Initialized data (counts toward both ROM and RAM)
- `.bss` - Uninitialized data (RAM only)
- `.rodata` - Read-only data in ROM

### ARM Release Build

The release build is optimized for size (`-Os`) and includes the following optimizations:
- Function sections (`-ffunction-sections`)
- Data sections (`-fdata-sections`)
- Garbage collection of unused sections (`-Wl,--gc-sections`)
- No RTTI and no exceptions for C++ code

| Component | text (ROM) | data (RAM) | bss (RAM) | Total |
|-----------|------------|------------|-----------|-------|
| **Total Libraries** | Generated by CI | Generated by CI | Generated by CI | Generated by CI |

### Understanding Memory Usage

**ROM (Flash Memory):**
- `.text` section: Executable code
- `.rodata` section: Constant data
- `.data` section: Initial values for initialized variables

**RAM (SRAM):**
- `.data` section: Initialized global/static variables
- `.bss` section: Uninitialized global/static variables
- Stack and heap (not shown in these statistics)

The actual binary statistics are generated automatically by the [GitHub Actions workflow](.github/workflows/build.yml) for each build. View the workflow runs to see the latest size information.

## CI/CD

The project includes GitHub Actions workflow for:
- ARM GCC cross-compilation (Debug and Release)
- Native builds
- Unit testing with GoogleTest
- Python script validation

## Target Device

Default target: **STM32F205** (Cortex-M3)
- CPU: ARM Cortex-M3
- FPU: Software floating point
- Memory: Configurable via linker script

## License

This project uses the SEGGER RTT library, which is automatically fetched from the [official SEGGER RTT repository](https://github.com/SEGGERMicro/RTT) and is available under the SEGGER RTT License. The SEGGER RTT library is used without modification.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Ensure all tests pass
5. Submit a pull request

## References

- [SEGGER RTT](https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/)
- [GoogleTest](https://github.com/google/googletest)
- [FreeRTOS](https://www.freertos.org/)
- [STM32F2 Series](https://www.st.com/en/microcontrollers-microprocessors/stm32f2-series.html)

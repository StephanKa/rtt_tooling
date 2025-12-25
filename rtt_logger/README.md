# RTT Logger

A modern C++ logger library using SEGGER Real-Time Transfer (RTT) for embedded systems debugging and diagnostics.

## Features

- **Modern C++17/20/23** with type-safe interfaces
- **Multiple log levels**: Trace, Debug, Info, Warning, Error, Critical
- **Printf-style formatted logging** with compile-time type checking (C++20)
- **Zero-copy string_view** support for efficient string handling
- **Configurable log level filtering** at runtime
- **Minimal overhead** - optimized for embedded systems
- **Thread-safe** RTT output
- **C++20 concepts** for enhanced type safety (when available)

## Requirements

- C++17 or later (C++20 recommended for full feature set)
- SEGGER RTT library (automatically fetched by CMake)
- CMake 3.20 or later

## Integration

Add to your CMakeLists.txt:

```cmake
target_link_libraries(your_application
    PRIVATE
        rtt_logger
)
```

## Quick Start

```cpp
#include <rtt_logger/rtt_logger.hpp>

int main() {
    // Initialize RTT
    rtt::Logger::initialize();
    
    // Get global logger instance
    auto& logger = rtt::getLogger();
    logger.setMinLevel(rtt::LogLevel::Debug);
    
    // Log messages at different levels
    logger.trace("Detailed trace information");
    logger.debug("Debug information");
    logger.info("Application started");
    logger.warning("Warning message");
    logger.error("Error occurred");
    logger.critical("Critical failure!");
    
    // Formatted logging
    int value = 42;
    logger.logFormatted(rtt::LogLevel::Info, "Value: %d", value);
    
    return 0;
}
```

## API Reference

### Log Levels

```cpp
enum class LogLevel : uint8_t {
    Trace = 0,      // Most verbose - detailed debugging
    Debug,          // General debugging information
    Info,           // Informational messages
    Warning,        // Warning messages
    Error,          // Error messages
    Critical        // Critical failures
};
```

### Logger Class

#### Initialization

```cpp
// Initialize RTT system
static void Logger::initialize();

// Get global logger instance
auto& logger = rtt::getLogger();
```

#### Configuration

```cpp
// Set minimum log level (messages below this level are filtered)
logger.setMinLevel(rtt::LogLevel::Info);

// Get current minimum log level
LogLevel level = logger.getMinLevel();

// Check if a specific log level is enabled
bool enabled = logger.isEnabled(rtt::LogLevel::Debug);
```

#### Logging Methods

```cpp
// Simple string logging
logger.trace("Trace message");
logger.debug("Debug message");
logger.info("Info message");
logger.warning("Warning message");
logger.error("Error message");
logger.critical("Critical message");

// Printf-style formatted logging
logger.logFormatted(rtt::LogLevel::Info, "Value: %d, String: %s", 42, "text");

// Direct log with level
logger.log(rtt::LogLevel::Warning, "Custom level message");
```

## Examples

See the [examples](examples/) directory for complete examples:

- `logger_example.cpp` - Comprehensive demonstration of all logging features

### Building Examples

```bash
# Native build
cmake --preset default
cmake --build --preset default
./build/default/rtt_logger/logger_example

# ARM build
cmake --preset arm-stm32f205
cmake --build --preset arm-stm32f205
# Flash and view output via RTT
```

## C++ Standard Features

### C++17 (Baseline)
- `string_view` for efficient string handling
- `constexpr` for compile-time optimizations
- Type traits for template metaprogramming

### C++20 (Recommended)
- **Concepts** for compile-time type checking
  ```cpp
  template<typename T>
  concept Formattable = /* type constraints */;
  ```
- Enhanced `constexpr` support
- Safer type conversions

### C++23
- All C++20 features plus latest standard improvements

## Performance

- **Memory**: ~200 bytes RAM for logger instance
- **Code size**: ~2-4 KB flash (depends on optimization)
- **Speed**: Logging overhead <1µs per message (typical)
- **RTT Buffer**: Default 1024 bytes (configurable in SEGGER_RTT_Conf.h)

## Best Practices

1. **Initialize early**: Call `Logger::initialize()` before any logging
2. **Set appropriate level**: Use `setMinLevel()` to filter unnecessary messages
3. **Use appropriate levels**: 
   - Trace: Very detailed debugging
   - Debug: Development debugging
   - Info: Normal operation messages
   - Warning: Potential issues
   - Error: Errors that don't stop execution
   - Critical: Fatal errors
4. **Format carefully**: Use type-safe formatting in C++20 with concepts
5. **Avoid logging in ISRs**: RTT is generally safe but keep ISR logging minimal

## Troubleshooting

### No RTT output visible
1. Ensure `Logger::initialize()` is called
2. Check RTT viewer is connected to channel 0 (default log channel)
3. Verify debug probe connection
4. Check log level filtering - may be filtering all messages

### Garbled output
1. RTT buffer may be too small - increase buffer size
2. Check for concurrent access from interrupts
3. Verify correct CPU frequency in RTT viewer

## Reading RTT Output

### Using J-Link
```bash
# Using Python script
python3 scripts/rtt_reader.py --backend jlink --device STM32F205RB

# Using SEGGER RTT Viewer (GUI)
# Configure: Target device, RTT channel 0
```

### Using OpenOCD
```bash
# Start OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg

# In another terminal, read RTT
python3 scripts/rtt_reader.py --backend openocd --host localhost --port 4444
```

## License

This library uses the SEGGER RTT library, available under the SEGGER RTT License.

## See Also

- [Main README](../README.md) - Project overview
- [SEGGER RTT Documentation](https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/)
- [RTT Benchmark](../rtt_benchmark/) - Code performance measurement
- [RTT Unit Testing](../rtt_unittest/) - Unit testing with RTT output

# RTT Memory Dump

Memory dump utilities for debugging and analysis via SEGGER RTT in embedded systems.

## Features

- **Multiple dump formats** - Hex, HexAscii, Binary, Decimal
- **Flexible configuration** - Customizable bytes per line, address display
- **Type-safe dumping** - Template-based object dumping
- **C++20 span support** - Modern interface for memory regions
- **Address/offset display** - Show memory addresses and offsets
- **ASCII representation** - View printable characters alongside hex
- **RTT output** - Real-time memory inspection without halting
- **Minimal overhead** - Efficient memory access and formatting

## Requirements

- C++17 or later (C++20 recommended for span support)
- RTT Logger library
- SEGGER RTT library
- CMake 3.20 or later

## Integration

Add to your CMakeLists.txt:

```cmake
target_link_libraries(your_application
    PRIVATE
        rtt_memory_dump
        rtt_logger
)
```

## Quick Start

```cpp
#include <rtt_memory_dump/rtt_memory_dump.hpp>
#include <rtt_logger/rtt_logger.hpp>

int main() {
    // Initialize RTT
    rtt::Logger::initialize();
    
    // Create memory dumper
    rtt::memory_dump::MemoryDumper dumper;
    
    // Dump a buffer
    uint8_t buffer[64];
    dumper.dump(buffer, sizeof(buffer), "My Buffer");
    
    // Dump a structure
    struct MyData {
        uint32_t id;
        float value;
    } data = {42, 3.14f};
    
    dumper.dumpObject(data, "My Data");
    
    return 0;
}
```

## API Reference

### DumpFormat Enumeration

```cpp
enum class DumpFormat : uint8_t {
    Hex,        // Hexadecimal only: "DE AD BE EF"
    HexAscii,   // Hex with ASCII: "DE AD BE EF | ....@"
    Binary,     // Binary: "11011110 10101101"
    Decimal     // Decimal: "222 173 190 239"
};
```

### DumpConfig Structure

```cpp
struct DumpConfig {
    DumpFormat format = DumpFormat::HexAscii;
    size_t bytes_per_line = 16;      // Bytes displayed per line
    bool show_address = true;         // Show memory addresses
    bool show_offset = false;         // Show offset from start
};
```

### MemoryDumper Class

```cpp
class MemoryDumper {
public:
    // Constructors
    explicit MemoryDumper(Logger& logger = rtt::getLogger());
    explicit MemoryDumper(const DumpConfig& config, Logger& logger = rtt::getLogger());
    
    // Configuration
    void setConfig(const DumpConfig& config);
    const DumpConfig& getConfig() const;
    
    // Memory dumping
    void dump(const void* data, size_t size, std::string_view description = "");
    
    // Object dumping (template)
    template<typename T>
    void dumpObject(const T& object, std::string_view description = "");
    
#if __cplusplus >= 202002L
    // C++20 span interface
    void dump(std::span<const uint8_t> data, std::string_view description = "");
#endif
};
```

## Usage Examples

### Basic Memory Dump

```cpp
rtt::memory_dump::MemoryDumper dumper;

uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
dumper.dump(data, sizeof(data), "Test Data");
```

**Output:**
```
=== Memory Dump: Test Data ===
Address: 0x20001000, Size: 8 bytes
0x20001000: DE AD BE EF CA FE BA BE | ........
=== End Memory Dump ===
```

### Dump Structures

```cpp
struct SensorData {
    uint32_t timestamp;
    float temperature;
    float pressure;
} __attribute__((packed));

SensorData sensor = {0x12345678, 23.5f, 1013.25f};
dumper.dumpObject(sensor, "Sensor Reading");
```

**Output:**
```
=== Memory Dump: Sensor Reading ===
Address: 0x20001100, Size: 12 bytes
0x20001100: 78 56 34 12 00 00 BC 41 00 00 7E 44 | xV4....A..~D
=== End Memory Dump ===
```

### Custom Format - Hex Only

```cpp
rtt::memory_dump::DumpConfig config(rtt::memory_dump::DumpFormat::Hex);
rtt::memory_dump::MemoryDumper dumper(config);

dumper.dump(buffer, size, "Hex Only");
```

**Output:**
```
=== Memory Dump: Hex Only ===
Address: 0x20001200, Size: 16 bytes
0x20001200: 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10
=== End Memory Dump ===
```

### Custom Format - Binary

```cpp
rtt::memory_dump::DumpConfig config(rtt::memory_dump::DumpFormat::Binary);
config.bytes_per_line = 4;  // 4 bytes per line
rtt::memory_dump::MemoryDumper dumper(config);

dumper.dump(flags, sizeof(flags), "Flag Register");
```

**Output:**
```
=== Memory Dump: Flag Register ===
Address: 0x20001300, Size: 4 bytes
0x20001300: 11010111 10101010 11110000 00001111
=== End Memory Dump ===
```

### Custom Configuration

```cpp
rtt::memory_dump::DumpConfig config;
config.format = rtt::memory_dump::DumpFormat::HexAscii;
config.bytes_per_line = 8;       // 8 bytes per line
config.show_address = true;
config.show_offset = true;

rtt::memory_dump::MemoryDumper dumper(config);
dumper.dump(data, size, "Custom Format");
```

### C++20 Span Interface

```cpp
#if __cplusplus >= 202002L
std::span<const uint8_t> dataSpan(buffer, buffer_size);
dumper.dump(dataSpan, "Span Data");
#endif
```

### Runtime Configuration Change

```cpp
rtt::memory_dump::MemoryDumper dumper;

// Initial dump with default format
dumper.dump(data1, size1, "Data 1");

// Change to hex-only
rtt::memory_dump::DumpConfig hexConfig(rtt::memory_dump::DumpFormat::Hex);
dumper.setConfig(hexConfig);
dumper.dump(data2, size2, "Data 2");

// Change to binary
rtt::memory_dump::DumpConfig binConfig(rtt::memory_dump::DumpFormat::Binary);
dumper.setConfig(binConfig);
dumper.dump(data3, size3, "Data 3");
```

## Dump Format Examples

### HexAscii (Default)
```
0x20001000: 48 65 6C 6C 6F 20 57 6F 72 6C 64 21 00 00 00 00 | Hello World!....
```

### Hex
```
0x20001000: 48 65 6C 6C 6F 20 57 6F 72 6C 64 21 00 00 00 00
```

### Binary
```
0x20001000: 01001000 01100101 01101100 01101100
0x20001004: 01101111 00100000 01010111 01101111
```

### Decimal
```
0x20001000: 72 101 108 108 111 32 87 111 114 108 100 33 0 0 0 0
```

## Examples

See the [examples](examples/) directory for complete examples:

- `memory_dump_example.cpp` - Comprehensive demonstration of memory dumping

### Building Examples

```bash
# Native build
cmake --preset default
cmake --build --preset default
./build/default/rtt_memory_dump/memory_dump_example

# ARM build
cmake --preset arm-stm32f205
cmake --build --preset arm-stm32f205
# Flash and view output via RTT
```

## Use Cases

### Debugging Structures

```cpp
// Inspect structure contents
struct ConfigBlock {
    uint32_t magic;
    uint16_t version;
    uint8_t flags;
    uint8_t reserved;
} config;

dumper.dumpObject(config, "Config Block");
```

### Analyzing Buffers

```cpp
// Check buffer contents after DMA transfer
uint8_t rxBuffer[256];
dumper.dump(rxBuffer, sizeof(rxBuffer), "DMA RX Buffer");
```

### Verifying Flash Contents

```cpp
// Verify flash write
const void* flashAddr = (void*)0x08010000;
dumper.dump(flashAddr, 128, "Flash Sector");
```

### Checking Packet Data

```cpp
// Inspect network packet
struct Packet {
    uint8_t header[8];
    uint8_t payload[64];
} packet;

dumper.dumpObject(packet, "Network Packet");
```

## Best Practices

1. **Use descriptive names**: Help identify dumps in RTT output
2. **Appropriate format**: Choose format based on data type
   - Binary: Flags and bit fields
   - Hex: General-purpose debugging
   - HexAscii: Text and mixed data
   - Decimal: Numeric data
3. **Limit dump size**: Large dumps can overflow RTT buffer
4. **Alignment**: Use `__attribute__((packed))` for structures
5. **Volatile data**: Be aware data may change during dump

## Performance Considerations

### Memory Usage
- MemoryDumper class: ~100 bytes
- Temporary buffers: ~100 bytes during dump
- RTT buffer: Depends on dump size

### Speed
- Formatting overhead: ~10-50µs per line (typical)
- RTT transmission: ~1µs per byte (typical)
- Large dumps may take several milliseconds

### RTT Buffer
- Default buffer: 1024 bytes
- Large dumps may need buffer flushing
- Consider dumping in chunks for very large regions

## Troubleshooting

### Truncated dumps
1. Increase RTT buffer size in SEGGER_RTT_Conf.h
2. Reduce bytes_per_line
3. Dump in smaller chunks
4. Add delays between dumps

### No output
1. Verify Logger::initialize() is called
2. Check RTT viewer connection
3. Ensure memory region is accessible
4. Check for memory access faults

### Garbled output
1. Verify data alignment
2. Check for concurrent RTT access
3. Increase RTT buffer size
4. Reduce dump frequency

## Reading RTT Output

### Using J-Link
```bash
python3 scripts/rtt_reader.py --backend jlink --device STM32F205RB
```

### Using OpenOCD
```bash
# Start OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg

# Read memory dumps
python3 scripts/rtt_reader.py --backend openocd --host localhost --port 4444
```

## See Also

- [Main README](../README.md) - Project overview
- [RTT Logger](../rtt_logger/) - Core logging functionality
- [RTT Data](../rtt_data/) - Structured data transmission

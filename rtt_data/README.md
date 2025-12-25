# RTT Data Transmission

Generic data transmission library for sending structured data via SEGGER RTT from embedded devices to host PC.

## Features

- **Type-safe interface** - Send integers, floats, strings, and binary data
- **Automatic type identification** - Data tagged with type information
- **Optional timestamping** - Add timestamps to data for time-series analysis
- **Multiple data types** - Support for all common data types
- **Custom structures** - Send any trivially copyable struct
- **C++20 span support** - Modern interface for binary data
- **Binary protocol** - Efficient data transmission
- **Host-side reader** - Python script to receive and decode data

## Requirements

- C++17 or later (C++20 recommended for span support)
- RTT Logger library
- SEGGER RTT library
- CMake 3.20 or later
- Python 3.6+ for host-side data reader

## Integration

Add to your CMakeLists.txt:

```cmake
target_link_libraries(your_application
    PRIVATE
        rtt_data
        rtt_logger
)
```

## Quick Start

### Embedded Side (Device)

```cpp
#include <rtt_data/rtt_data.hpp>
#include <rtt_logger/rtt_logger.hpp>

int main() {
    // Initialize RTT
    rtt::Logger::initialize();
    
    // Get global data sender (uses RTT channel 1)
    auto& dataSender = rtt::data::getDataSender();
    
    // Send different data types
    dataSender.sendInt(static_cast<int32_t>(42));
    dataSender.sendFloat(3.14159f);
    dataSender.sendString("Hello from device!");
    
    // Send binary data
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    dataSender.sendBinary(data, sizeof(data));
    
    return 0;
}
```

### Host Side (PC)

```bash
# Read data via J-Link
python3 scripts/rtt_data_reader.py --backend jlink --device STM32F205RB --channel 1

# Read data via OpenOCD
python3 scripts/rtt_data_reader.py --backend openocd --host localhost --port 4444 --channel 1
```

**Output:**
```
Reading RTT data from channel 1...
[Int32] 42
[Float] 3.141590
[String] "Hello from device!"
[Binary] 0xdeadbeef
```

## API Reference

### DataSender Class

```cpp
class DataSender {
public:
    // Constructor
    explicit DataSender(unsigned int channel = 1, Logger& logger = rtt::getLogger());
    
    // Integer types
    void sendInt(int8_t value);
    void sendInt(uint8_t value);
    void sendInt(int16_t value);
    void sendInt(uint16_t value);
    void sendInt(int32_t value);
    void sendInt(uint32_t value);
    void sendInt(int64_t value);
    void sendInt(uint64_t value);
    
    // Floating-point types
    void sendFloat(float value);
    void sendFloat(double value);
    
    // String data
    void sendString(std::string_view str);
    
    // Binary data
    void sendBinary(const void* data, size_t size);
    
#if __cplusplus >= 202002L
    // C++20 span interface
    void sendBinary(std::span<const uint8_t> data);
#endif
    
    // Generic send (for trivially copyable types)
    template<typename T>
    void send(const T& value);
    
    // Timestamping control
    void setTimestamping(bool enabled);
    bool isTimestampingEnabled() const;
};

// Get global data sender instance
DataSender& getDataSender();
```

## Usage Examples

### Sending Integers

```cpp
auto& sender = rtt::data::getDataSender();

// Different integer sizes
sender.sendInt(static_cast<int8_t>(127));
sender.sendInt(static_cast<uint8_t>(255));
sender.sendInt(static_cast<int16_t>(-1000));
sender.sendInt(static_cast<uint16_t>(50000));
sender.sendInt(static_cast<int32_t>(-100000));
sender.sendInt(static_cast<uint32_t>(4000000000U));
sender.sendInt(static_cast<int64_t>(-9000000000LL));
sender.sendInt(static_cast<uint64_t>(18000000000ULL));
```

### Sending Floating-Point

```cpp
// Float (32-bit)
sender.sendFloat(3.14159f);
sender.sendFloat(2.71828f);

// Double (64-bit)
sender.sendFloat(3.141592653589793);
sender.sendFloat(2.718281828459045);
```

### Sending Strings

```cpp
sender.sendString("System initialized");
sender.sendString("Temperature: 23.5C");

// String view for efficiency
std::string_view message = "Status OK";
sender.sendString(message);
```

### Sending Binary Data

```cpp
// Raw bytes
uint8_t buffer[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE};
sender.sendBinary(buffer, sizeof(buffer));

// C++20 span
#if __cplusplus >= 202002L
std::span<const uint8_t> dataSpan(buffer, sizeof(buffer));
sender.sendBinary(dataSpan);
#endif
```

### Sending Custom Structures

```cpp
// Structure must be trivially copyable
struct SensorData {
    float temperature;
    float humidity;
    uint32_t pressure;
} __attribute__((packed));

SensorData sensor = {23.5f, 65.2f, 101325};
sender.send(sensor);  // Generic send
```

### Timestamped Data

```cpp
auto& sender = rtt::data::getDataSender();

// Enable timestamping
sender.setTimestamping(true);

// Send time-series data
for (int i = 0; i < 100; ++i) {
    int32_t value = readSensor();
    sender.sendInt(value);
    delay(10);  // 10ms between samples
}

// Disable timestamping
sender.setTimestamping(false);
```

### Sensor Data Stream

```cpp
void sendSensorReadings() {
    auto& sender = rtt::data::getDataSender();
    sender.setTimestamping(true);
    
    while (true) {
        // Read sensors
        float temp = readTemperature();
        float humidity = readHumidity();
        uint32_t pressure = readPressure();
        
        // Send data
        sender.sendFloat(temp);
        sender.sendFloat(humidity);
        sender.sendInt(pressure);
        
        delay(1000);  // 1 second interval
    }
}
```

## Data Protocol

### Data Packet Format

Each data packet consists of:
1. Type identifier (1 byte)
2. Optional timestamp (4 bytes, if timestamping enabled)
3. Data payload (variable size)

### Type Identifiers

| Type      | ID   | Size     |
|-----------|------|----------|
| Int8      | 0x01 | 1 byte   |
| UInt8     | 0x02 | 1 byte   |
| Int16     | 0x03 | 2 bytes  |
| UInt16    | 0x04 | 2 bytes  |
| Int32     | 0x05 | 4 bytes  |
| UInt32    | 0x06 | 4 bytes  |
| Int64     | 0x07 | 8 bytes  |
| UInt64    | 0x08 | 8 bytes  |
| Float     | 0x10 | 4 bytes  |
| Double    | 0x11 | 8 bytes  |
| String    | 0x20 | Variable |
| Binary    | 0x30 | Variable |

## Examples

See the [examples](examples/) directory for complete examples:

- `data_example.cpp` - Comprehensive demonstration of data transmission

### Building Examples

```bash
# Native build
cmake --preset default
cmake --build --preset default
./build/default/rtt_data/data_example

# ARM build
cmake --preset arm-stm32f205
cmake --build --preset arm-stm32f205
# Flash and view data via Python reader
```

## Host-Side Data Reader

### Basic Usage

```bash
# Read from J-Link
python3 scripts/rtt_data_reader.py --backend jlink --device STM32F205RB --channel 1

# Read from OpenOCD
python3 scripts/rtt_data_reader.py --backend openocd --host localhost --port 4444 --channel 1

# Save to file
python3 scripts/rtt_data_reader.py --backend jlink --device STM32F205RB --channel 1 --output data.bin

# Parse binary file
python3 scripts/rtt_data_reader.py --file data.bin

# Verbose output
python3 scripts/rtt_data_reader.py --backend jlink --device STM32F205RB --channel 1 --verbose
```

### Output Formats

**Default output:**
```
[Int32] 42
[Float] 3.141590
[String] "Hello World"
[Binary] 0xdeadbeef
```

**With timestamps:**
```
[000001] [Int32] 100
[000002] [Int32] 200
[000003] [Float] 23.5
```

## Use Cases

### Telemetry Data

```cpp
// Send periodic telemetry
void sendTelemetry() {
    auto& sender = rtt::data::getDataSender();
    
    sender.sendInt(getSystemUptime());
    sender.sendFloat(getCpuLoad());
    sender.sendInt(getFreeMemory());
}
```

### Debug Data Logging

```cpp
// Log debug values
void logDebugData(int errorCode, const char* message) {
    auto& sender = rtt::data::getDataSender();
    
    sender.sendInt(static_cast<int32_t>(errorCode));
    sender.sendString(message);
}
```

### Performance Profiling

```cpp
// Send timing data
sender.setTimestamping(true);
for (int i = 0; i < iterations; ++i) {
    uint32_t startTime = getTimestamp();
    performOperation();
    uint32_t elapsed = getTimestamp() - startTime;
    sender.sendInt(elapsed);
}
```

### Sensor Data Acquisition

```cpp
// Stream sensor data to PC
while (true) {
    SensorData data = readAllSensors();
    sender.send(data);
    delay(100);
}
```

## Best Practices

1. **Use appropriate channel**: Default channel 1 (channel 0 for logs)
2. **Type consistency**: Use correct type casts for integers
3. **Structure packing**: Use `__attribute__((packed))` for structures
4. **Rate limiting**: Don't overflow RTT buffer with high-rate data
5. **Timestamping**: Enable for time-series analysis
6. **Binary for efficiency**: Use binary format for large data
7. **Error handling**: Check RTT buffer availability for critical data

## Performance Considerations

### Bandwidth
- RTT bandwidth: ~100-1000 KB/s (depends on probe and configuration)
- Overhead: ~1-5 bytes per packet (type + optional timestamp)
- Large structures: Use binary format for efficiency

### Buffer Management
- Default RTT channel 1 buffer: 1024 bytes
- Monitor buffer usage for high-rate data
- Consider increasing buffer size if needed

### Timing
- sendInt/sendFloat: ~2-5µs (typical)
- sendString: ~5-20µs (depends on length)
- sendBinary: ~10-100µs (depends on size)

## Troubleshooting

### No data received on host
1. Verify correct RTT channel (default: 1)
2. Check device is running and RTT initialized
3. Ensure debug probe is connected
4. Verify Python script is using correct backend/device

### Corrupted data
1. Check RTT buffer isn't overflowing
2. Verify structure packing on device
3. Ensure consistent endianness
4. Increase RTT buffer size if needed

### Missed data packets
1. Reduce data transmission rate
2. Increase RTT buffer size
3. Use binary format for large data
4. Check for buffer overflow on device

## See Also

- [Main README](../README.md) - Project overview
- [RTT Logger](../rtt_logger/) - Core logging functionality
- [RTT Memory Dump](../rtt_memory_dump/) - Memory inspection
- [Python Scripts](../scripts/) - Host-side data readers

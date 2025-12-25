#include <rtt_logger/rtt_logger.hpp>
#include <rtt_memory_dump/rtt_memory_dump.hpp>
#include <cstdint>
#include <array>

// Example structures to demonstrate memory dumping
struct SensorData
{
    uint32_t timestamp;
    float temperature;
    float pressure;
    uint16_t humidity;
    uint8_t status;
    uint8_t reserved;
};

struct DeviceConfig
{
    uint32_t device_id;
    uint8_t mode;
    uint8_t flags;
    uint16_t timeout_ms;
    uint32_t baud_rate;
    char name[16];
};

int main()
{
    // Initialize RTT
    rtt::Logger::initialize();

    // Get global logger instance
    auto& logger = rtt::getLogger();
    logger.setMinLevel(rtt::LogLevel::Info);

    logger.info("===========================================");
    logger.info("  RTT Memory Dump Example");
    logger.info("===========================================");

    // Example 1: Basic memory dump with default hex+ASCII format
    {
        logger.info("");
        logger.info("Example 1: Hex+ASCII dump of sensor data");
        logger.info("-------------------------------------------");

        SensorData sensor = {
            .timestamp = 0x12345678,
            .temperature = 23.5f,
            .pressure = 1013.25f,
            .humidity = 65,
            .status = 0xA5,
            .reserved = 0x00
        };

        rtt::memory_dump::MemoryDumper dumper(logger);
        dumper.dumpObject(sensor, "SensorData structure");
    }

    // Example 2: Memory dump in hexadecimal only
    {
        logger.info("");
        logger.info("Example 2: Hex-only dump of device config");
        logger.info("-------------------------------------------");

        DeviceConfig config = {
            .device_id = 0xDEADBEEF,
            .mode = 0x01,
            .flags = 0xF0,
            .timeout_ms = 1000,
            .baud_rate = 115200,
            .name = "MyDevice"
        };

        rtt::memory_dump::DumpConfig cfg(rtt::memory_dump::DumpFormat::Hex);
        rtt::memory_dump::MemoryDumper dumper(cfg, logger);
        dumper.dumpObject(config, "DeviceConfig structure");
    }

    // Example 3: Memory dump in binary format
    {
        logger.info("");
        logger.info("Example 3: Binary dump of byte array");
        logger.info("-------------------------------------------");

        std::array<uint8_t, 8> flags = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

        rtt::memory_dump::DumpConfig cfg(rtt::memory_dump::DumpFormat::Binary);
        cfg.bytes_per_line = 4;
        rtt::memory_dump::MemoryDumper dumper(cfg, logger);
        dumper.dump(flags.data(), flags.size(), "Flag bits");
    }

    // Example 4: Memory dump in decimal format
    {
        logger.info("");
        logger.info("Example 4: Decimal dump of integer array");
        logger.info("-------------------------------------------");

        std::array<uint8_t, 10> values = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

        rtt::memory_dump::MemoryDumper dumper(logger);
        dumper.setFormat(rtt::memory_dump::DumpFormat::Decimal);
        dumper.setBytesPerLine(5);
        dumper.dump(values.data(), values.size(), "Decimal values");
    }

    // Example 5: Dumping arbitrary memory region
    {
        logger.info("");
        logger.info("Example 5: Dumping stack memory region");
        logger.info("-------------------------------------------");

        char message[] = "Hello RTT Memory Dump!";

        rtt::memory_dump::MemoryDumper dumper(logger);
        dumper.dump(message, sizeof(message), "Message string");
    }

    // Example 6: Custom configuration with different bytes per line
    {
        logger.info("");
        logger.info("Example 6: Custom 8 bytes per line dump");
        logger.info("-------------------------------------------");

        std::array<uint32_t, 8> data = {
            0x11111111, 0x22222222, 0x33333333, 0x44444444,
            0x55555555, 0x66666666, 0x77777777, 0x88888888
        };

        rtt::memory_dump::DumpConfig cfg;
        cfg.bytes_per_line = 8;
        cfg.format = rtt::memory_dump::DumpFormat::HexAscii;

        rtt::memory_dump::MemoryDumper dumper(cfg, logger);
        dumper.dump(data.data(), sizeof(data), "32-bit integer array");
    }

#if __cplusplus >= 202002L
    // Example 7: C++20 span-based dump
    {
        logger.info("");
        logger.info("Example 7: C++20 span-based memory dump");
        logger.info("-------------------------------------------");

        std::array<uint8_t, 16> buffer = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
        };

        rtt::memory_dump::MemoryDumper dumper(logger);
        std::span<const uint8_t> buffer_span(buffer);
        dumper.dump(buffer_span, "Sequential byte pattern");
    }
#endif

    logger.info("");
    logger.info("===========================================");
    logger.info("  Memory Dump Examples Completed");
    logger.info("===========================================");

    return 0;
}

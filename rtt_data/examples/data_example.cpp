#include <rtt_data/rtt_data.hpp>
#include <rtt_logger/rtt_logger.hpp>

/**
 * Example demonstrating RTT data transmission
 *
 * This example shows how to send various data types via RTT
 * using the generic DataSender interface.
 */
int main()
{
    // Initialize RTT logger
    rtt::Logger::initialize();
    auto& logger = rtt::getLogger();
    logger.setMinLevel(rtt::LogLevel::Info);

    logger.info("RTT Data Example Started");

    // Get the global data sender (uses RTT channel 1 by default)
    auto& dataSender = rtt::data::getDataSender();

    // Example 1: Send integers of different sizes
    logger.info("Sending integers...");
    dataSender.sendInt(static_cast<int8_t>(42));
    dataSender.sendInt(static_cast<uint8_t>(255));
    dataSender.sendInt(static_cast<int16_t>(-1000));
    dataSender.sendInt(static_cast<uint16_t>(50000));
    dataSender.sendInt(-100000);
    dataSender.sendInt(4000000000U);
    dataSender.sendInt(-9000000000LL);
    dataSender.sendInt(18000000000ULL);

    // Example 2: Send floating-point values
    logger.info("Sending floating-point values...");
    dataSender.sendFloat(3.14159f);
    dataSender.sendFloat(2.71828);

    // Example 3: Send strings
    logger.info("Sending strings...");
    dataSender.sendString("Hello from RTT!");
    dataSender.sendString("Data transmission test");

    // Example 4: Send binary data
    logger.info("Sending binary data...");
    uint8_t binaryData[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
    dataSender.sendBinary(binaryData, sizeof(binaryData));

    // Example 5: Enable timestamping and send data
    logger.info("Sending data with timestamps...");
    dataSender.setTimestamping(true);
    for (int i = 0; i < 10; ++i)
    {
        dataSender.sendInt(static_cast<int32_t>(i * 100));
    }

    // Example 6: Send custom struct (must be trivially copyable)
    struct SensorData
    {
        float temperature;
        float humidity;
        uint32_t pressure;
    } __attribute__((packed));

    logger.info("Sending custom struct...");
    const SensorData sensor = {23.5f, 65.2f, 101325};
    dataSender.send(sensor);

#if __cplusplus >= 202002L
    // Example 7: C++20 span interface
    logger.info("Sending data via span (C++20)...");
    std::span<const uint8_t> dataSpan(binaryData, sizeof(binaryData));
    dataSender.sendBinary(dataSpan);
#endif

    logger.info("RTT Data Example Completed");

    return 0;
}

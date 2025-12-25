#include <gtest/gtest.h>
#include "rtt_unittest/rtt_unittest.hpp"
#include "rtt_logger/rtt_logger.hpp"

/**
 * Custom main function for unit tests that outputs via RTT
 *
 * This replaces the default gtest_main and redirects all GoogleTest
 * output to RTT for embedded debugging.
 */
int main(int argc, char** argv)
{
    // Initialize RTT
    rtt::Logger::initialize();

    // Get logger instance
    auto& logger = rtt::getLogger();
    logger.setMinLevel(rtt::LogLevel::Trace);

    // Initialize GoogleTest
    ::testing::InitGoogleTest(&argc, argv);

    // Install RTT event listener
    rtt::unittest::InstallRttTestListener(logger, true);

    logger.info("RTT Unit Test Framework Initialized");
    logger.info("All test output will be sent via RTT");

    // Run all tests
    int result = RUN_ALL_TESTS();

    logger.logFormatted(rtt::LogLevel::Info, "Test run complete with exit code: %d", result);

    return result;
}

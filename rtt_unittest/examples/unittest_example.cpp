#include <rtt_unittest/rtt_unittest.hpp>
#include <rtt_logger/rtt_logger.hpp>

/**
 * Example demonstrating RTT unit testing capabilities
 *
 * This example shows how to use the RTT capture functionality
 * to test RTT output in unit tests. For full GoogleTest integration,
 * see the tests/ directory.
 */

int main()
{
    // Initialize RTT logger
    rtt::Logger::initialize();
    auto& logger = rtt::getLogger();
    logger.setMinLevel(rtt::LogLevel::Info);

    logger.info("===========================================");
    logger.info("  RTT Unit Testing Example");
    logger.info("===========================================");
    logger.info("");

    // Demonstrate RTT capture for testing
    logger.info("Example 1: RTT Output Capture");
    logger.info("-------------------------------------------");

    rtt::unittest::RttCapture capture;

    // Start capturing RTT output
    capture.startCapture();

    // Generate some log messages
    logger.info("First test message");
    logger.warning("Second test message");
    logger.error("Third test message");

    // Stop capturing
    capture.stopCapture();

    // Check captured output
    logger.info("");
    logger.logFormatted(rtt::LogLevel::Info, "Captured %zu messages", capture.getMessageCount());

    if (capture.containsMessage("First test message"))
    {
        logger.info("✓ Found 'First test message'");
    }
    else
    {
        logger.error("✗ Did not find 'First test message'");
    }

    // Example 2: Using ScopedRttCapture (RAII)
    logger.info("");
    logger.info("Example 2: Scoped RTT Capture");
    logger.info("-------------------------------------------");

    capture.clear();

    {
        rtt::unittest::ScopedRttCapture scopedCapture(capture);
        // All messages in this scope are captured
        logger.debug("Scoped message 1");
        logger.debug("Scoped message 2");
    } // Capture stops automatically

    logger.logFormatted(rtt::LogLevel::Info, "Captured %zu scoped messages", capture.getMessageCount());

    // Example 3: Testing specific log output
    logger.info("");
    logger.info("Example 3: Verifying Specific Output");
    logger.info("-------------------------------------------");

    capture.clear();
    capture.startCapture();

    // Function under test
    auto testFunction = []()
    {
        auto& log = rtt::getLogger();
        log.info("Initializing system...");
        log.debug("Debug: Configuration loaded");
        log.info("System ready");
    };

    testFunction();
    capture.stopCapture();

    if (capture.containsMessage("Initializing system...") &&
        capture.containsMessage("System ready"))
    {
        logger.info("✓ Function produced expected output");
    }
    else
    {
        logger.error("✗ Function did not produce expected output");
    }

    // Example 4: Checking last message
    logger.info("");
    logger.info("Example 4: Last Message Check");
    logger.info("-------------------------------------------");

    capture.clear();
    capture.startCapture();
    logger.info("Message 1");
    logger.info("Message 2");
    logger.info("Last message");
    capture.stopCapture();

    std::string lastMsg = capture.getLastMessage();
    logger.logFormatted(rtt::LogLevel::Info, "Last captured message: %s", lastMsg.c_str());

    if (lastMsg == "Last message")
    {
        logger.info("✓ Last message matches expected");
    }
    else
    {
        logger.error("✗ Last message does not match");
    }

    logger.info("");
    logger.info("===========================================");
    logger.info("  RTT Unit Testing Example Completed");
    logger.info("===========================================");
    logger.info("");
    logger.info("Note: For full GoogleTest integration:");
    logger.info("  - See tests/test_rtt_logger.cpp");
    logger.info("  - See tests/test_rtt_unittest.cpp");
    logger.info("  - Build with BUILD_TESTING=ON");

    return 0;
}

#include <rtt_logger/rtt_logger.hpp>

/**
 * Example demonstrating RTT logger usage
 *
 * This example shows how to use the RTT logger with different
 * log levels and formatting options.
 */
int main()
{
    // Initialize RTT
    rtt::Logger::initialize();

    // Get global logger instance
    auto& logger = rtt::getLogger();

    // Set minimum log level (only messages at this level or higher will be output)
    logger.setMinLevel(rtt::LogLevel::Trace);

    logger.info("===========================================");
    logger.info("  RTT Logger Example");
    logger.info("===========================================");

    // Example 1: Basic logging at different levels
    logger.trace("This is a TRACE message - very detailed debugging");
    logger.debug("This is a DEBUG message - general debugging info");
    logger.info("This is an INFO message - informational output");
    logger.warning("This is a WARNING message - something to watch");
    logger.error("This is an ERROR message - something went wrong");
    logger.critical("This is a CRITICAL message - serious problem!");

    // Example 2: Formatted logging with printf-style formatting
    const int value = 42;
    const float pi = 3.14159f;
    const auto str = "formatted";

    logger.logFormatted(rtt::LogLevel::Info, "Integer value: %d", value);
    logger.logFormatted(rtt::LogLevel::Info, "Float value: %.2f", pi);
    logger.logFormatted(rtt::LogLevel::Info, "String value: %s", str);
    logger.logFormatted(rtt::LogLevel::Info, "Multiple values: %d, %.3f, %s", value, pi, str);

    // Example 3: Adjusting log level filtering
    logger.info("Setting minimum level to Warning - Trace/Debug/Info will be filtered");
    logger.setMinLevel(rtt::LogLevel::Warning);

    logger.trace("This TRACE won't be shown");
    logger.debug("This DEBUG won't be shown");
    logger.info("This INFO won't be shown");
    logger.warning("But this WARNING will be shown");
    logger.error("And this ERROR will be shown");

    // Example 4: Check if a log level is enabled
    logger.setMinLevel(rtt::LogLevel::Info);
    if (logger.isEnabled(rtt::LogLevel::Debug))
    {
        logger.debug("Debug logging is enabled");
    }
    else
    {
        logger.info("Debug logging is disabled");
    }

    // Example 5: Log level query
    auto currentLevel = logger.getMinLevel();
    logger.logFormatted(rtt::LogLevel::Info, "Current minimum level: %d", static_cast<int>(currentLevel));

#if __cplusplus >= 202002L
    // Example 6: C++20 concepts ensure type safety at compile time
    logger.info("Using C++20 with concepts for enhanced type safety");

    // These will work:
    logger.logFormatted(rtt::LogLevel::Info, "Integer: %d", 123);
    logger.logFormatted(rtt::LogLevel::Info, "Float: %f", 3.14);

    // For string literals, use a variable or const char* cast
    const auto text = "text";
    logger.logFormatted(rtt::LogLevel::Info, "String: %s", text);

    // Non-formattable types would cause compile errors with concepts
#endif

    logger.info("===========================================");
    logger.info("  Logger Example Completed");
    logger.info("===========================================");

    return 0;
}

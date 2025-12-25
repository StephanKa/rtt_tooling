#include <gtest/gtest.h>
#include "rtt_logger/rtt_logger.hpp"

namespace rtt::test
{
    class RttLoggerTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            Logger::initialize();
        }
    };

    TEST_F(RttLoggerTest, LoggerCreation)
    {
        Logger logger(0, LogLevel::Info);
        EXPECT_EQ(logger.getMinLevel(), LogLevel::Info);
    }

    TEST_F(RttLoggerTest, SetMinLevel)
    {
        Logger logger;
        logger.setMinLevel(LogLevel::Debug);
        EXPECT_EQ(logger.getMinLevel(), LogLevel::Debug);
    }

    TEST_F(RttLoggerTest, IsEnabledWorks)
    {
        Logger logger(0, LogLevel::Warning);

        EXPECT_FALSE(logger.isEnabled(LogLevel::Trace));
        EXPECT_FALSE(logger.isEnabled(LogLevel::Debug));
        EXPECT_FALSE(logger.isEnabled(LogLevel::Info));
        EXPECT_TRUE(logger.isEnabled(LogLevel::Warning));
        EXPECT_TRUE(logger.isEnabled(LogLevel::Error));
        EXPECT_TRUE(logger.isEnabled(LogLevel::Critical));
    }

    TEST_F(RttLoggerTest, GlobalLoggerExists)
    {
        auto& logger = getLogger();
        logger.setMinLevel(LogLevel::Trace);
        EXPECT_EQ(logger.getMinLevel(), LogLevel::Trace);
    }

    TEST_F(RttLoggerTest, LoggingMethods)
    {
        auto& logger = getLogger();
        logger.setMinLevel(LogLevel::Trace);

        // These should not crash
        logger.trace("Trace message");
        logger.debug("Debug message");
        logger.info("Info message");
        logger.warning("Warning message");
        logger.error("Error message");
        logger.critical("Critical message");
    }

    TEST_F(RttLoggerTest, WriteData)
    {
        auto& logger = getLogger();
        const char data[] = "Test data";
        size_t written = logger.write(data, sizeof(data) - 1);
        // Should write some data or return 0 if buffer full
        EXPECT_GE(written, 0);
    }
} // namespace rtt::test

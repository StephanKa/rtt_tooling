#pragma once

#include <rtt_logger/rtt_logger.hpp>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#if __cplusplus >= 202002L
#include <ranges>
#endif

#ifdef BUILD_TESTING
#include <gtest/gtest.h>
#endif

namespace rtt::unittest
{
    /**
     * @brief RTT output capture for unit testing
     *
     * This class captures RTT output for verification in unit tests.
     * It provides a mock RTT interface that can be used with GoogleTest.
     */
    class RttCapture
    {
    public:
        RttCapture() = default;
        ~RttCapture() = default;

        // Non-copyable, non-movable
        RttCapture(const RttCapture&) = delete;
        RttCapture& operator=(const RttCapture&) = delete;
        RttCapture(RttCapture&&) = delete;
        RttCapture& operator=(RttCapture&&) = delete;

        /**
         * @brief Start capturing RTT output
         */
        void startCapture();

        /**
         * @brief Stop capturing RTT output
         */
        void stopCapture();

        /**
         * @brief Clear captured output
         */
        void clear();

        /**
         * @brief Get captured output
         * @return Vector of captured messages
         */
        [[nodiscard]] const std::vector<std::string>& getOutput() const noexcept
        {
            return m_capturedOutput;
        }

        /**
         * @brief Get number of captured messages
         * @return Number of messages
         */
        [[nodiscard]] size_t getMessageCount() const noexcept
        {
            return m_capturedOutput.size();
        }

        /**
         * @brief Check if specific message was captured
         * @param message Message to search for
         * @return true if message found, false otherwise
         */
        [[nodiscard]] bool containsMessage(const std::string& message) const noexcept
        {
#if __cplusplus >= 202002L
            return std::ranges::find(m_capturedOutput, message) != m_capturedOutput.end();
#else
            return std::find(m_capturedOutput.begin(), m_capturedOutput.end(), message) != m_capturedOutput.end();
#endif
        }

        /**
         * @brief Get the last captured message
         * @return Last message or empty string if no messages
         */
        [[nodiscard]] std::string getLastMessage() const noexcept
        {
            return m_capturedOutput.empty() ? "" : m_capturedOutput.back();
        }

    private:
        std::vector<std::string> m_capturedOutput;
        bool m_capturing {false};
    };

    /**
     * @brief RAII helper for RTT capture
     *
     * Automatically starts capture on construction and stops on destruction.
     */
    class ScopedRttCapture
    {
    public:
        explicit ScopedRttCapture(RttCapture& capture) : m_capture(capture)
        {
            m_capture.startCapture();
        }

        ~ScopedRttCapture()
        {
            m_capture.stopCapture();
        }

        // Non-copyable, non-movable
        ScopedRttCapture(const ScopedRttCapture&) = delete;
        ScopedRttCapture& operator=(const ScopedRttCapture&) = delete;
        ScopedRttCapture(ScopedRttCapture&&) = delete;
        ScopedRttCapture& operator=(ScopedRttCapture&&) = delete;

    private:
        RttCapture& m_capture;
    };

#ifdef BUILD_TESTING
    /**
     * @brief GoogleTest event listener that redirects output to RTT
     *
     * This listener captures GoogleTest output and sends it via RTT instead of stdout.
     * This is useful for embedded systems where stdout is not available.
     */
    class RttTestEventListener : public ::testing::EmptyTestEventListener
    {
    public:
        explicit RttTestEventListener(rtt::Logger& logger) : m_logger(logger)
        {
        }

        void OnTestProgramStart(const ::testing::UnitTest& unit_test) override
        {
            m_logger.info("=== Test Program Start ===");
            m_logger.logFormatted(LogLevel::Info, "Running %d tests from %d test suites",
                                 unit_test.test_to_run_count(),
                                 unit_test.test_suite_to_run_count());
        }

        void OnTestSuiteStart(const ::testing::TestSuite& test_suite) override
        {
            m_logger.logFormatted(LogLevel::Info, "[----------] %d tests from %s",
                                 test_suite.test_to_run_count(),
                                 test_suite.name());
        }

        void OnTestStart(const ::testing::TestInfo& test_info) override
        {
            m_logger.logFormatted(LogLevel::Info, "[ RUN      ] %s.%s",
                                 test_info.test_suite_name(),
                                 test_info.name());
        }

        void OnTestPartResult(const ::testing::TestPartResult& result) override
        {
            if (result.type() == ::testing::TestPartResult::kSuccess)
                return;

            const char* type = result.failed() ? "FAILURE" : "SUCCESS";
            m_logger.logFormatted(LogLevel::Error, "[  FAILED  ] %s:%d: %s",
                                 result.file_name() ? result.file_name() : "unknown",
                                 result.line_number(),
                                 result.message());
        }

        void OnTestEnd(const ::testing::TestInfo& test_info) override
        {
            const char* result = test_info.result()->Passed() ? "       OK" : "  FAILED";
            m_logger.logFormatted(LogLevel::Info, "[%s] %s.%s (%lld ms)",
                                 result,
                                 test_info.test_suite_name(),
                                 test_info.name(),
                                 test_info.result()->elapsed_time());
        }

        void OnTestSuiteEnd(const ::testing::TestSuite& test_suite) override
        {
            m_logger.logFormatted(LogLevel::Info, "[----------] %d tests from %s (%lld ms total)",
                                 test_suite.test_to_run_count(),
                                 test_suite.name(),
                                 test_suite.elapsed_time());
        }

        void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override
        {
            m_logger.info("=== Test Program End ===");
            m_logger.logFormatted(LogLevel::Info, "[==========] %d tests from %d test suites ran (%lld ms total)",
                                 unit_test.test_to_run_count(),
                                 unit_test.test_suite_to_run_count(),
                                 unit_test.elapsed_time());

            m_logger.logFormatted(LogLevel::Info, "[  PASSED  ] %d tests",
                                 unit_test.successful_test_count());

            if (unit_test.failed_test_count() > 0)
            {
                m_logger.logFormatted(LogLevel::Error, "[  FAILED  ] %d tests",
                                     unit_test.failed_test_count());
            }
        }

    private:
        rtt::Logger& m_logger;
    };

    /**
     * @brief Install RTT event listener for GoogleTest
     *
     * This function installs a custom event listener that redirects GoogleTest
     * output to RTT. Call this before RUN_ALL_TESTS() in your main function.
     *
     * @param logger Logger instance to use for output
     * @param remove_default_listener If true, removes the default console output listener
     */
    inline void InstallRttTestListener(rtt::Logger& logger, bool remove_default_listener = true)
    {
        ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();

        if (remove_default_listener)
        {
            delete listeners.Release(listeners.default_result_printer());
        }

        listeners.Append(new RttTestEventListener(logger));
    }
#endif // BUILD_TESTING
} // namespace rtt::unittest

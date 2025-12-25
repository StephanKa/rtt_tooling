#include <rtt_logger/rtt_logger.hpp>
#include "SEGGER_RTT.h"

namespace rtt
{
    void Logger::log(LogLevel level, std::string_view message) noexcept
    {
        if (!isEnabled(level))
        {
            return;
        }

        const char* levelStr = getLevelString(level);

        // Write level prefix
        SEGGER_RTT_WriteString(m_channel, levelStr);
        SEGGER_RTT_WriteString(m_channel, " ");

        // Write message (ensure null-terminated or use Write with size)
        SEGGER_RTT_Write(m_channel, message.data(), message.size());
        SEGGER_RTT_WriteString(m_channel, "\r\n");
    }

    size_t Logger::write(const void* data, size_t size) const noexcept
    {
        return SEGGER_RTT_Write(m_channel, data, size);
    }

    bool Logger::initialize() noexcept
    {
        SEGGER_RTT_Init();
        return true;
    }

    // Global logger instance
    static Logger g_logger{0, LogLevel::Info};

    Logger& getLogger() noexcept
    {
        return g_logger;
    }
} // namespace rtt

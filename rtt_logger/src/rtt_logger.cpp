#include <rtt_logger/rtt_logger.hpp>
#include "SEGGER_RTT.h"

namespace rtt
{
    // Global logger instance — Info level by default
    static Logger g_logger{0, LogLevel::Info};

    Logger& getLogger() noexcept
    {
        return g_logger;
    }
} // namespace rtt

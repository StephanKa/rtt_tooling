#pragma once

#include <cstdint>
#include <string_view>
#include <type_traits>

#if __cplusplus >= 202002L
#include <concepts>
#endif

// Forward declare SEGGER RTT functions
extern "C" {
int SEGGER_RTT_printf(unsigned int BufferIndex, const char* sFormat, ...);
unsigned int SEGGER_RTT_WriteString(unsigned int BufferIndex, const char* s);
}

namespace rtt
{
    /**
     * @brief Log levels for RTT logger
     */
    enum class LogLevel : uint8_t
    {
        Trace = 0,
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

#if __cplusplus >= 202002L
    // C++20 Concept for formattable types (safe for printf-style formatting)
    template <typename T>
    concept Formattable = std::is_arithmetic_v<std::remove_cvref_t<T>> ||
        std::same_as<std::remove_cvref_t<T>, const char*> ||
        std::same_as<std::remove_cvref_t<T>, char*> ||
        std::same_as<std::remove_cvref_t<T>, void*> ||
        std::same_as<std::remove_cvref_t<T>, const void*>;
#endif

    /**
     * @brief Modern C++ wrapper for SEGGER RTT logging
     *
     * This class provides a type-safe, modern C++ interface for RTT logging
     * using C++17/C++20 features like constexpr, string_view, concepts, and variadic templates.
     */
    class Logger
    {
    public:
        /**
         * @brief Construct a new Logger object
         * @param channel RTT channel number (default: 0)
         * @param level Minimum log level to output (default: Info)
         */
        explicit constexpr Logger(uint32_t channel = 0, LogLevel level = LogLevel::Info) noexcept
            : m_channel(channel), m_minLevel(level)
        {
        }

        /**
         * @brief Set the minimum log level
         * @param level New minimum log level
         */
        constexpr void setMinLevel(LogLevel level) noexcept
        {
            m_minLevel = level;
        }

        /**
         * @brief Get the current minimum log level
         * @return Current minimum log level
         */
        [[nodiscard]] constexpr LogLevel getMinLevel() const noexcept
        {
            return m_minLevel;
        }

        /**
         * @brief Check if a log level is enabled
         * @param level Log level to check
         * @return true if enabled, false otherwise
         */
        [[nodiscard]] constexpr bool isEnabled(LogLevel level) const noexcept
        {
            return level >= m_minLevel;
        }

        /**
         * @brief Log a message with specified level
         * @param level Log level
         * @param message Message to log
         */
        void log(LogLevel level, std::string_view message) noexcept;

        /**
         * @brief Log a formatted message with specified level
         * @param level Log level
         * @param format Format string
         * @param args Format arguments
         */
#if __cplusplus >= 202002L
        template <Formattable... Args>
        void logFormatted(LogLevel level, const char* format, Args&&... args) noexcept;
#else
        template <typename... Args>
        void logFormatted(LogLevel level, const char* format, Args&&... args) noexcept;
#endif

        /**
         * @brief Log trace message
         */
        void trace(std::string_view message) noexcept
        {
            log(LogLevel::Trace, message);
        }

        /**
         * @brief Log debug message
         */
        void debug(std::string_view message) noexcept
        {
            log(LogLevel::Debug, message);
        }

        /**
         * @brief Log info message
         */
        void info(std::string_view message) noexcept
        {
            log(LogLevel::Info, message);
        }

        /**
         * @brief Log warning message
         */
        void warning(std::string_view message) noexcept
        {
            log(LogLevel::Warning, message);
        }

        /**
         * @brief Log error message
         */
        void error(std::string_view message) noexcept
        {
            log(LogLevel::Error, message);
        }

        /**
         * @brief Log critical message
         */
        void critical(std::string_view message) noexcept
        {
            log(LogLevel::Critical, message);
        }

        /**
         * @brief Write raw data to RTT buffer
         * @param data Pointer to data
         * @param size Size of data in bytes
         * @return Number of bytes written
         */
        [[nodiscard]] size_t write(const void* data, size_t size) const noexcept;

        /**
         * @brief Initialize RTT
         * @return true if successful, false otherwise
         */
        static bool initialize() noexcept;

    private:
        uint32_t m_channel;
        LogLevel m_minLevel;

        [[nodiscard]] static constexpr const char* getLevelString(LogLevel level) noexcept
        {
            switch (level)
            {
            case LogLevel::Trace:
                {
                    return "[TRACE]";
                }
            case LogLevel::Debug:
                {
                    return "[DEBUG]";
                }
            case LogLevel::Info:
                {
                    return "[INFO]";
                }
            case LogLevel::Warning:
                {
                    return "[WARN]";
                }
            case LogLevel::Error:
                {
                    return "[ERROR]";
                }
            case LogLevel::Critical:
                {
                    return "[CRIT]";
                }
            default:
                {
                    return "[UNKNOWN]";
                }
            }
        }
    };

    /**
     * @brief Get the global logger instance
     * @return Reference to global logger
     */
    Logger& getLogger() noexcept;

    // Template implementation
#if __cplusplus >= 202002L
    template <Formattable... Args>
#else
    template <typename... Args>

#endif
    void Logger::logFormatted(LogLevel level, const char* format, Args&&... args) noexcept
    {
        if (!isEnabled(level))
        {
            return;
        }

        const char* levelStr = getLevelString(level);
        SEGGER_RTT_WriteString(m_channel, levelStr);
        SEGGER_RTT_WriteString(m_channel, " ");

        SEGGER_RTT_printf(m_channel, format, std::forward<Args>(args)...);
        SEGGER_RTT_WriteString(m_channel, "\r\n");
    }
} // namespace rtt

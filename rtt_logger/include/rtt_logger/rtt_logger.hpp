#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <expected>
#include <string_view>
#include <type_traits>
#include <utility>

// Forward declare SEGGER RTT functions
extern "C" {
int SEGGER_RTT_printf(unsigned int BufferIndex, const char* sFormat, ...);
unsigned int SEGGER_RTT_WriteString(unsigned int BufferIndex, const char* s);
unsigned int SEGGER_RTT_Write(unsigned int BufferIndex, const void* pBuffer, unsigned int NumBytes);
void SEGGER_RTT_Init(void);
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

    /// C++23 concept: types safe for printf-style formatting
    template <typename T>
    concept Formattable = std::is_arithmetic_v<std::remove_cvref_t<T>> ||
        std::same_as<std::remove_cvref_t<T>, const char*> ||
        std::same_as<std::remove_cvref_t<T>, char*> ||
        std::same_as<std::remove_cvref_t<T>, void*> ||
        std::same_as<std::remove_cvref_t<T>, const void*>;

    /**
     * @brief Modern C++23 RTT logger with compile-time log level gating.
     *
     * The MinLevel template parameter sets a compile-time floor: any log call with a
     * level below MinLevel compiles to nothing — zero binary footprint on that call site.
     *
     * @tparam MinLevel Compile-time minimum log level (default: Trace = log everything).
     *
     * Example — strip Debug/Info from a production binary at compile time:
     * @code
     *   using ProdLogger = BasicLogger<LogLevel::Warning>;
     * @endcode
     */
    template <LogLevel MinLevel = LogLevel::Trace>
    class BasicLogger
    {
    public:
        explicit constexpr BasicLogger(uint32_t channel = 0, LogLevel level = MinLevel) noexcept
            : m_channel(channel), m_minLevel(level)
        {
        }

        constexpr void setMinLevel(LogLevel level) noexcept { m_minLevel = level; }
        [[nodiscard]] constexpr LogLevel getMinLevel() const noexcept { return m_minLevel; }
        [[nodiscard]] constexpr bool isEnabled(LogLevel level) const noexcept { return level >= m_minLevel; }

        // ── Compile-time level overloads — zero cost when Level < MinLevel ────

        template <LogLevel Level>
        void log(std::string_view message) const noexcept
        {
            if constexpr (Level >= MinLevel)
            {
                if (isEnabled(Level))
                    logImpl(Level, message);
            }
        }

        template <LogLevel Level, Formattable... Args>
        void logFormatted(const char* format, Args&&... args) noexcept
        {
            if constexpr (Level >= MinLevel)
            {
                if (isEnabled(Level))
                {
                    SEGGER_RTT_WriteString(m_channel, getLevelString(Level));
                    SEGGER_RTT_WriteString(m_channel, " ");
                    SEGGER_RTT_printf(m_channel, format, std::forward<Args>(args)...);
                    SEGGER_RTT_WriteString(m_channel, "\r\n");
                }
            }
        }

        // ── Runtime level overloads — for dynamic level selection ─────────────

        void log(LogLevel level, std::string_view message) const noexcept
        {
            if (level >= MinLevel && isEnabled(level))
                logImpl(level, message);
        }

        template <Formattable... Args>
        void logFormatted(LogLevel level, const char* format, Args&&... args) noexcept
        {
            if (level >= MinLevel && isEnabled(level))
            {
                SEGGER_RTT_WriteString(m_channel, getLevelString(level));
                SEGGER_RTT_WriteString(m_channel, " ");
                SEGGER_RTT_printf(m_channel, format, std::forward<Args>(args)...);
                SEGGER_RTT_WriteString(m_channel, "\r\n");
            }
        }

        // ── Convenience wrappers ──────────────────────────────────────────────

        void trace(std::string_view message) const noexcept { log<LogLevel::Trace>(message); }
        void debug(std::string_view message) const noexcept { log<LogLevel::Debug>(message); }
        void info(std::string_view message) const noexcept { log<LogLevel::Info>(message); }
        void warning(std::string_view message) const noexcept { log<LogLevel::Warning>(message); }
        void error(std::string_view message) const noexcept { log<LogLevel::Error>(message); }
        void critical(std::string_view message) const noexcept { log<LogLevel::Critical>(message); }

        [[nodiscard]] size_t write(const void* data, size_t size) const noexcept
        {
            return SEGGER_RTT_Write(m_channel, data, static_cast<unsigned>(size));
        }

        /// Initialize RTT. Returns an empty expected on success, or an error string.
        static std::expected<void, const char*> initialize() noexcept
        {
            SEGGER_RTT_Init();
            return {};
        }

    private:
        uint32_t m_channel;
        LogLevel m_minLevel;

        void logImpl(LogLevel level, std::string_view message) const noexcept
        {
            SEGGER_RTT_WriteString(m_channel, getLevelString(level));
            SEGGER_RTT_WriteString(m_channel, " ");
            SEGGER_RTT_Write(m_channel, message.data(), static_cast<unsigned>(message.size()));
            SEGGER_RTT_WriteString(m_channel, "\r\n");
        }

        [[nodiscard]] static constexpr const char* getLevelString(LogLevel level) noexcept
        {
            switch (level)
            {
            case LogLevel::Trace: return "[TRACE]";
            case LogLevel::Debug: return "[DEBUG]";
            case LogLevel::Info: return "[INFO]";
            case LogLevel::Warning: return "[WARN]";
            case LogLevel::Error: return "[ERROR]";
            case LogLevel::Critical: return "[CRIT]";
            }
            std::unreachable();
        }
    };

    /// Default logger type — runtime m_minLevel is the sole gate; all levels active.
    using Logger = BasicLogger<>;

    /**
     * @brief Get the global logger instance
     * @return Reference to global logger
     */
    Logger& getLogger() noexcept;
} // namespace rtt

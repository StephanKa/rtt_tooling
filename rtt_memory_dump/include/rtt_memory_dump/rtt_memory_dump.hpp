#pragma once

#include <rtt_logger/rtt_logger.hpp>
#include <cstdint>
#include <string_view>

#if __cplusplus >= 202002L
#include <span>
#endif

namespace rtt::memory_dump
{
    /**
     * @brief Memory dump format options
     */
    enum class DumpFormat : uint8_t
    {
        Hex, // Hexadecimal format (default)
        HexAscii, // Hexadecimal with ASCII representation
        Binary, // Binary format (0s and 1s)
        Decimal // Decimal format
    };

    /**
     * @brief Configuration for memory dump output
     */
    struct DumpConfig
    {
        DumpFormat format = DumpFormat::HexAscii;
        size_t bytes_per_line = 16; // Number of bytes per line
        bool show_address = true; // Show memory addresses
        bool show_offset = false; // Show offset from start address

        constexpr DumpConfig() noexcept = default;

        explicit constexpr DumpConfig(DumpFormat fmt, size_t bytes_line = 16,
                                      bool addr = true, bool offset = false) noexcept
            : format(fmt), bytes_per_line(bytes_line),
              show_address(addr), show_offset(offset)
        {
        }
    };

    /**
     * @brief Memory dumper class for dumping memory regions via RTT
     *
     * This class provides functionality to dump memory regions in various
     * formats via RTT for debugging and analysis purposes.
     */
    class MemoryDumper
    {
    public:
        /**
         * @brief Construct a new Memory Dumper object
         * @param logger Logger instance to use (defaults to global logger)
         */
        explicit MemoryDumper(Logger& logger = rtt::getLogger()) noexcept
            : m_logger(logger)
        {
        }

        /**
         * @brief Construct a new Memory Dumper object with custom configuration
         * @param config Dump configuration
         * @param logger Logger instance to use
         */
        explicit MemoryDumper(const DumpConfig& config, Logger& logger = rtt::getLogger()) noexcept
            : m_logger(logger), m_config(config)
        {
        }

        /**
         * @brief Dump a memory region via RTT
         * @param data Pointer to memory region to dump
         * @param size Size of memory region in bytes
         * @param description Optional description of the memory region
         */
        void dump(const void* data, size_t size, std::string_view description = "") noexcept;

#if __cplusplus >= 202002L
        /**
         * @brief Dump a memory region via RTT (C++20 span version)
         * @param data Span of bytes to dump
         * @param description Optional description of the memory region
         */
        void dump(std::span<const uint8_t> data, std::string_view description = "") noexcept;
#endif

        /**
         * @brief Dump a typed object's memory representation
         * @tparam T Type of object to dump
         * @param obj Reference to object to dump
         * @param description Optional description
         */
        template <typename T>
        void dumpObject(const T& obj, std::string_view description = "") noexcept;

        /**
         * @brief Set dump configuration
         * @param config New configuration
         */
        void setConfig(const DumpConfig& config) noexcept
        {
            m_config = config;
        }

        /**
         * @brief Get current dump configuration
         * @return Current configuration
         */
        [[nodiscard]] constexpr const DumpConfig& getConfig() const noexcept
        {
            return m_config;
        }

        /**
         * @brief Set dump format
         * @param format New format
         */
        void setFormat(DumpFormat format) noexcept
        {
            m_config.format = format;
        }

        /**
         * @brief Set bytes per line
         * @param bytes Number of bytes per line
         */
        void setBytesPerLine(size_t bytes) noexcept
        {
            if (bytes > 0 && bytes <= 64)
            {
                m_config.bytes_per_line = bytes;
            }
        }

    private:
        Logger& m_logger;
        DumpConfig m_config;

        /**
         * @brief Format a single line of memory dump
         * @param data Pointer to data
         * @param size Size of data
         * @param address Base address
         * @param offset Offset from base address
         */
        void formatLine(const uint8_t* data, size_t size,
                        uintptr_t address, size_t offset) noexcept;

        /**
         * @brief Format bytes as hexadecimal
         * @param data Pointer to data
         * @param size Size of data
         * @param buffer Output buffer
         * @param buffer_size Size of output buffer
         */
        void formatHex(const uint8_t* data, size_t size,
                       char* buffer, size_t buffer_size) noexcept;

        /**
         * @brief Format bytes as ASCII (printable chars or '.')
         * @param data Pointer to data
         * @param size Size of data
         * @param buffer Output buffer
         * @param buffer_size Size of output buffer
         */
        void formatAscii(const uint8_t* data, size_t size,
                         char* buffer, size_t buffer_size) noexcept;

        /**
         * @brief Format bytes as binary
         * @param data Pointer to data
         * @param size Size of data
         * @param buffer Output buffer
         * @param buffer_size Size of output buffer
         */
        void formatBinary(const uint8_t* data, size_t size,
                          char* buffer, size_t buffer_size) noexcept;

        /**
         * @brief Format bytes as decimal
         * @param data Pointer to data
         * @param size Size of data
         * @param buffer Output buffer
         * @param buffer_size Size of output buffer
         */
        void formatDecimal(const uint8_t* data, size_t size,
                           char* buffer, size_t buffer_size) noexcept;
    };

    // Template implementation

    template <typename T>
    void MemoryDumper::dumpObject(const T& obj, std::string_view description) noexcept
    {
        const auto data = reinterpret_cast<const uint8_t*>(&obj);
        constexpr size_t size = sizeof(T);
        dump(data, size, description);
    }
} // namespace rtt::memory_dump

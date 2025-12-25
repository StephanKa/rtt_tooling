#include <rtt_memory_dump/rtt_memory_dump.hpp>
#include <SEGGER_RTT.h>
#include <cstdio>
#include <algorithm>

namespace rtt::memory_dump
{
    void MemoryDumper::dump(const void* data, size_t size, std::string_view description) noexcept
    {
        if (data == nullptr || size == 0)
        {
            m_logger.warning("Invalid memory dump request: null pointer or zero size");
            return;
        }

        const auto bytes = static_cast<const uint8_t*>(data);
        const uintptr_t address = reinterpret_cast<uintptr_t>(data);

        // Print header
        if (!description.empty())
        {
            m_logger.logFormatted(LogLevel::Info, "=== Memory Dump: %.*s ===",
                                  static_cast<int>(description.length()), description.data());
        }
        else
        {
            m_logger.info("=== Memory Dump ===");
        }

        m_logger.logFormatted(LogLevel::Info, "Address: 0x%08lX, Size: %zu bytes",
                              static_cast<unsigned long>(address), size);

        // Dump memory line by line
        size_t offset = 0;
        while (offset < size)
        {
            size_t line_size = std::min(m_config.bytes_per_line, size - offset);
            formatLine(bytes + offset, line_size, address, offset);
            offset += line_size;
        }

        m_logger.info("=== End Memory Dump ===");
    }

#if __cplusplus >= 202002L
    void MemoryDumper::dump(std::span<const uint8_t> data, std::string_view description) noexcept
    {
        dump(data.data(), data.size(), description);
    }
#endif

    void MemoryDumper::formatLine(const uint8_t* data, size_t size,
                                  uintptr_t address, size_t offset) noexcept
    {
        static constexpr size_t BUFFER_SIZE = 512;
        char buffer[BUFFER_SIZE];
        size_t pos = 0;

        // Format address or offset
        if (m_config.show_address)
        {
            pos += snprintf(buffer + pos, BUFFER_SIZE - pos,
                            "0x%08lX: ",
                            static_cast<unsigned long>(address + offset));
        }
        else if (m_config.show_offset)
        {
            pos += snprintf(buffer + pos, BUFFER_SIZE - pos,
                            "+0x%04zX: ", offset);
        }

        // Format data based on selected format
        switch (m_config.format)
        {
        case DumpFormat::Hex:
            formatHex(data, size, buffer + pos, BUFFER_SIZE - pos);
            break;

        case DumpFormat::HexAscii:
            {
                // Format hex part
                char hex_buffer[256];
                formatHex(data, size, hex_buffer, sizeof(hex_buffer));

                // Pad hex part to align ASCII
                const size_t expected_hex_len = m_config.bytes_per_line * 3;
                pos += snprintf(buffer + pos, BUFFER_SIZE - pos, "%-*s",
                                static_cast<int>(expected_hex_len), hex_buffer);

                // Add separator
                pos += snprintf(buffer + pos, BUFFER_SIZE - pos, " | ");

                // Format ASCII part
                formatAscii(data, size, buffer + pos, BUFFER_SIZE - pos);
                break;
            }

        case DumpFormat::Binary:
            formatBinary(data, size, buffer + pos, BUFFER_SIZE - pos);
            break;

        case DumpFormat::Decimal:
            formatDecimal(data, size, buffer + pos, BUFFER_SIZE - pos);
            break;
        }

        // Output the formatted line
        m_logger.log(LogLevel::Info, buffer);
    }

    void MemoryDumper::formatHex(const uint8_t* data, size_t size,
                                 char* buffer, size_t buffer_size) noexcept
    {
        size_t pos = 0;
        for (size_t i = 0; i < size; ++i)
        {
            // Need space for "XX " (3 chars) plus null terminator
            if (pos + 4 > buffer_size)
            {
                break;
            }

            const int written = snprintf(buffer + pos, buffer_size - pos, "%02X ", data[i]);
            if (written > 0)
            {
                pos += written;
            }
        }
        // Remove trailing space and ensure null termination
        if (pos > 0 && pos < buffer_size && buffer[pos - 1] == ' ')
        {
            buffer[pos - 1] = '\0';
        }
        else if (pos < buffer_size)
        {
            buffer[pos] = '\0';
        }
    }

    void MemoryDumper::formatAscii(const uint8_t* data, size_t size,
                                   char* buffer, size_t buffer_size) noexcept
    {
        size_t pos = 0;
        for (size_t i = 0; i < size && pos < buffer_size - 1; ++i)
        {
            const char c = static_cast<char>(data[i]);
            // Print printable ASCII characters, otherwise print '.'
            buffer[pos++] = (c >= 32 && c <= 126) ? c : '.';
        }

        // Pad remaining bytes if line is not full
        while (pos < m_config.bytes_per_line && pos < buffer_size - 1)
        {
            buffer[pos++] = ' ';
        }

        buffer[pos] = '\0';
    }

    void MemoryDumper::formatBinary(const uint8_t* data, size_t size,
                                    char* buffer, size_t buffer_size) noexcept
    {
        size_t pos = 0;
        for (size_t i = 0; i < size; ++i)
        {
            // Need space for 8 bits + 1 space + null terminator
            if (pos + 10 > buffer_size)
            {
                break;
            }

            for (int bit = 7; bit >= 0; --bit)
            {
                buffer[pos++] = (data[i] & (1 << bit)) ? '1' : '0';
            }
            buffer[pos++] = ' ';
        }
        // Remove trailing space and ensure null termination
        if (pos > 0 && pos < buffer_size && buffer[pos - 1] == ' ')
        {
            buffer[pos - 1] = '\0';
        }
        else if (pos < buffer_size)
        {
            buffer[pos] = '\0';
        }
    }

    void MemoryDumper::formatDecimal(const uint8_t* data, size_t size,
                                     char* buffer, size_t buffer_size) noexcept
    {
        size_t pos = 0;
        for (size_t i = 0; i < size; ++i)
        {
            // Need space for "NNN " (4 chars) plus null terminator
            if (pos + 5 > buffer_size)
            {
                break;
            }

            const int written = snprintf(buffer + pos, buffer_size - pos, "%3u ", data[i]);
            if (written > 0)
            {
                pos += written;
            }
        }
        // Remove trailing space and ensure null termination
        if (pos > 0 && pos < buffer_size && buffer[pos - 1] == ' ')
        {
            buffer[pos - 1] = '\0';
        }
        else if (pos < buffer_size)
        {
            buffer[pos] = '\0';
        }
    }
} // namespace rtt::memory_dump

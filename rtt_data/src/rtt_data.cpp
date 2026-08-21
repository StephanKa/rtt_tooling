#include "rtt_data/rtt_data.hpp"

#include <cstring>

namespace rtt::data
{
    // Global DataSender instance
    static DataSender g_dataSender;

    DataSender& getDataSender() noexcept
    {
        return g_dataSender;
    }

    uint32_t DataSender::calculateCrc32(const uint8_t* data, size_t size) noexcept
    {
        uint32_t crc = 0xFFFFFFFFU;
        for (size_t index = 0; index < size; ++index)
        {
            crc ^= data[index];
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                const uint32_t mask = 0U - (crc & 1U);
                crc = (crc >> 1U) ^ (0xEDB88320U & mask);
            }
        }
        return ~crc;
    }

    size_t DataSender::sendWithHeader(DataType type, const void* data, size_t size) noexcept
    {
        if ((data == nullptr && size != 0) || size > DATA_MAX_PAYLOAD_SIZE)
        {
            return 0;
        }

        // Create header
        DataHeader header{};
        header.magic[0] = DATA_MAGIC_0;
        header.magic[1] = DATA_MAGIC_1;
        header.version = DATA_PROTOCOL_VERSION;
        header.type = type;
        header.size = static_cast<uint32_t>(size);
        header.timestamp = getTimestamp();
        header.sequence = m_sequence++;
        header.payloadCrc32 = calculateCrc32(static_cast<const uint8_t*>(data), size);

        std::array<uint8_t, sizeof(DataHeader) + DATA_MAX_PAYLOAD_SIZE> packet{};
        std::memcpy(packet.data(), &header, sizeof(header));
        if (size != 0)
        {
            std::memcpy(packet.data() + sizeof(header), data, size);
        }

        const auto packetSize = static_cast<unsigned>(sizeof(header) + size);
        const auto sent = SEGGER_RTT_Write(m_channel, packet.data(), packetSize);
        return sent == packetSize ? sent : 0;
    }

    size_t DataSender::sendString(std::string_view str) noexcept
    {
        return sendWithHeader(DataType::String, str.data(), str.size());
    }

    size_t DataSender::sendBinary(const void* data, size_t size) noexcept
    {
        return sendWithHeader(DataType::Binary, data, size);
    }
} // namespace rtt::data

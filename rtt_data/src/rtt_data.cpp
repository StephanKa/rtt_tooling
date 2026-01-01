#include "rtt_data/rtt_data.hpp"

namespace rtt::data
{
    // Global DataSender instance
    static DataSender g_dataSender;

    DataSender& getDataSender() noexcept
    {
        return g_dataSender;
    }

    size_t DataSender::sendWithHeader(DataType type, const void* data, size_t size) noexcept
    {
        if ((data == nullptr) || size == 0)
        {
            return 0;
        }

        // Create header
        DataHeader header{};
        header.magic[0] = DATA_MAGIC_0;
        header.magic[1] = DATA_MAGIC_1;
        header.type = type;
        header.reserved = 0;
        header.size = static_cast<uint32_t>(size);
        header.timestamp = getTimestamp();

        // Send header
        size_t sent = SEGGER_RTT_Write(m_channel, &header, sizeof(header));

        // Send data
        sent += SEGGER_RTT_Write(m_channel, data, size);

        return sent;
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

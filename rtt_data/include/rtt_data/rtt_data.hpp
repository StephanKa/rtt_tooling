#pragma once

#include <cstdint>
#include <string_view>
#include <type_traits>

#if __cplusplus >= 202002L
#include <span>
#endif

// Forward declare SEGGER RTT functions
extern "C" {
unsigned int SEGGER_RTT_Write(unsigned int BufferIndex, const void* pBuffer, unsigned int NumBytes);
}

namespace rtt::data
{
    /**
     * @brief Data types for RTT data transmission
     */
    enum class DataType : uint8_t
    {
        Int8 = 0,
        UInt8,
        Int16,
        UInt16,
        Int32,
        UInt32,
        Int64,
        UInt64,
        Float,
        Double,
        String,
        Binary
    };

    /**
     * @brief Header for data packets sent via RTT
     */
    struct DataHeader
    {
        uint8_t magic[2]; // Magic bytes: 'R', 'D' (RTT Data)
        DataType type; // Data type
        uint8_t reserved; // Reserved for future use
        uint32_t size; // Data size in bytes
        uint32_t timestamp; // Timestamp (optional, 0 if not used)
    } __attribute__((packed));

    static constexpr uint8_t DATA_MAGIC_0 = 'R';
    static constexpr uint8_t DATA_MAGIC_1 = 'D';

#if __cplusplus >= 202002L
    // C++20 Concept for data types that can be sent
    template <typename T>
    concept Sendable = std::is_trivially_copyable_v<std::remove_cvref_t<T>>;
#endif

    /**
     * @brief Generic RTT data sender class
     *
     * This class provides a type-safe interface for sending structured data
     * via RTT to the host. Data is sent with headers that include type information,
     * allowing the host-side Python script to properly decode and format the data.
     */
    class DataSender
    {
    public:
        /**
         * @brief Construct a new DataSender object
         * @param channel RTT channel number for data transmission (default: 1)
         * @param use_timestamps Enable automatic timestamping (default: false)
         */
        explicit DataSender(uint32_t channel = 1, bool use_timestamps = false) noexcept
            : m_channel(channel), m_useTimestamps(use_timestamps), m_timestampCounter(0)
        {
        }

        /**
         * @brief Send an integer value
         * @tparam T Integer type (int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t)
         * @param value Value to send
         * @return Number of bytes sent
         */
#if __cplusplus >= 202002L
        template <typename T>
            requires std::is_integral_v<T>
        size_t sendInt(T value) noexcept;
#else
        template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
        size_t sendInt(T value) noexcept;
#endif

        /**
         * @brief Send a floating-point value
         * @tparam T Floating-point type (float or double)
         * @param value Value to send
         * @return Number of bytes sent
         */
#if __cplusplus >= 202002L
        template <typename T>
            requires std::is_floating_point_v<T>
        size_t sendFloat(T value) noexcept;
#else
        template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
        size_t sendFloat(T value) noexcept;
#endif

        /**
         * @brief Send a string
         * @param str String to send
         * @return Number of bytes sent
         */
        size_t sendString(std::string_view str) noexcept;

        /**
         * @brief Send binary data
         * @param data Pointer to data buffer
         * @param size Size of data in bytes
         * @return Number of bytes sent
         */
        size_t sendBinary(const void* data, size_t size) noexcept;

#if __cplusplus >= 202002L
        /**
         * @brief Send binary data (C++20 span interface)
         * @param data Span of data to send
         * @return Number of bytes sent
         */
        size_t sendBinary(std::span<const uint8_t> data) noexcept
        {
            return sendBinary(data.data(), data.size());
        }
#endif

        /**
         * @brief Send a generic trivially copyable type
         * @tparam T Type to send (must be trivially copyable)
         * @param value Value to send
         * @return Number of bytes sent
         */
#if __cplusplus >= 202002L
        template <typename T>
            requires Sendable<T>
        size_t send(const T& value) noexcept
        {
            return sendBinary(&value, sizeof(T));
        }
#else
        template <typename T>
        typename std::enable_if<std::is_trivially_copyable<T>::value, size_t>::type
        send(const T& value) noexcept
        {
            return sendBinary(&value, sizeof(T));
        }
#endif

        /**
         * @brief Enable or disable automatic timestamping
         * @param enable True to enable, false to disable
         */
        void setTimestamping(bool enable) noexcept
        {
            m_useTimestamps = enable;
        }

        /**
         * @brief Check if timestamping is enabled
         * @return True if enabled, false otherwise
         */
        [[nodiscard]] bool isTimestampingEnabled() const noexcept
        {
            return m_useTimestamps;
        }

        /**
         * @brief Get the current RTT channel
         * @return RTT channel number
         */
        [[nodiscard]] uint32_t getChannel() const noexcept
        {
            return m_channel;
        }

        /**
         * @brief Set the RTT channel
         * @param channel New channel number
         */
        void setChannel(uint32_t channel) noexcept
        {
            m_channel = channel;
        }

    private:
        uint32_t m_channel;
        bool m_useTimestamps;
        uint32_t m_timestampCounter;

        /**
         * @brief Get timestamp for current data packet
         * @return Timestamp value or 0 if disabled
         */
        uint32_t getTimestamp() noexcept
        {
            if (m_useTimestamps)
            {
                return m_timestampCounter++;
            }
            return 0;
        }

        /**
         * @brief Send data with header
         * @param type Data type
         * @param data Pointer to data
         * @param size Data size in bytes
         * @return Number of bytes sent (including header)
         */
        size_t sendWithHeader(DataType type, const void* data, size_t size) noexcept;

        /**
         * @brief Get DataType enum for integral types
         */
        template <typename T>
        static constexpr DataType getIntType() noexcept
        {
            if constexpr (std::is_same_v<T, int8_t>) return DataType::Int8;
            else if constexpr (std::is_same_v<T, uint8_t>) return DataType::UInt8;
            else if constexpr (std::is_same_v<T, int16_t>) return DataType::Int16;
            else if constexpr (std::is_same_v<T, uint16_t>) return DataType::UInt16;
            else if constexpr (std::is_same_v<T, int32_t>) return DataType::Int32;
            else if constexpr (std::is_same_v<T, uint32_t>) return DataType::UInt32;
            else if constexpr (std::is_same_v<T, int64_t>) return DataType::Int64;
            else if constexpr (std::is_same_v<T, uint64_t>) return DataType::UInt64;
            else return DataType::Binary; // Fallback
        }

        /**
         * @brief Get DataType enum for floating-point types
         */
        template <typename T>
        static constexpr DataType getFloatType() noexcept
        {
            if constexpr (std::is_same_v<T, float>) return DataType::Float;
            else if constexpr (std::is_same_v<T, double>) return DataType::Double;
            else return DataType::Binary; // Fallback
        }
    };

    /**
     * @brief Get global DataSender instance
     * @return Reference to global DataSender
     */
    DataSender& getDataSender() noexcept;

    // Template implementations
#if __cplusplus >= 202002L
    template <typename T>
        requires std::is_integral_v<T>
    size_t DataSender::sendInt(T value) noexcept
    {
        return sendWithHeader(getIntType<T>(), &value, sizeof(T));
    }

    template <typename T>
        requires std::is_floating_point_v<T>
    size_t DataSender::sendFloat(T value) noexcept
    {
        return sendWithHeader(getFloatType<T>(), &value, sizeof(T));
    }
#else
    template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type>
    size_t DataSender::sendInt(T value) noexcept
    {
        return sendWithHeader(getIntType<T>(), &value, sizeof(T));
    }

    template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type>
    size_t DataSender::sendFloat(T value) noexcept
    {
        return sendWithHeader(getFloatType<T>(), &value, sizeof(T));
    }
#endif
} // namespace rtt::data

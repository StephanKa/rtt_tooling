#include <rtt_freertos_trace/rtt_freertos_trace.hpp>
#include <SEGGER_RTT.h>
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <utility>

constexpr size_t MAX_TASK_NAME_LEN{16};
constexpr size_t MAX_REGISTERED_TASKS{32};
constexpr size_t RTT_TRACE_BUFFER_SIZE{2048}; // RTT up-buffer size for trace channel

/**
 * @brief Static RTT buffer for trace channel
 * This must be large enough to hold task registry text + binary events
 */
static char rtt_trace_buffer[RTT_TRACE_BUFFER_SIZE];

/**
 * @brief Task registry entry
 */
typedef struct
{
    uint32_t handle;
    char name[MAX_TASK_NAME_LEN];
} TaskRegistryEntry;

/**
 * @brief Trace state
 */
static struct
{
    uint8_t channel;
    TaskRegistryEntry task_registry[MAX_REGISTERED_TASKS];
    uint8_t num_registered_tasks;
} trace_state = {0, {}, 0};

static std::atomic_bool trace_initialized{false};
static std::atomic_bool trace_enabled{false};
static std::atomic<uint32_t> dropped_events{0};

static uint32_t calculate_crc32(const uint8_t* data, size_t size)
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

/**
 * @brief Get timestamp from FreeRTOS or system timer
 *
 * @note For ARM Cortex-M processors, uses DWT CYCCNT for high-resolution timestamps.
 *       For other platforms, returns 0 (platform-specific implementation required).
 */
uint32_t rtt_trace_get_timestamp()
{
#ifdef __ARM_ARCH
    // Use DWT CYCCNT register for high-resolution timestamp on ARM Cortex-M
    static volatile uint32_t* DWT_CYCCNT = (uint32_t*)0xE0001004;
    static volatile uint32_t* DWT_CONTROL = (uint32_t*)0xE0001000;
    static volatile uint32_t* SCB_DEMCR = (uint32_t*)0xE000EDFC;

    // Enable DWT if not already enabled
    static uint8_t dwt_initialized = 0;
    if (!dwt_initialized)
    {
        *SCB_DEMCR |= 0x01000000; // Enable trace
        *DWT_CYCCNT = 0; // Reset counter
        *DWT_CONTROL |= 1; // Enable counter
        dwt_initialized = 1;
    }

    return *DWT_CYCCNT;
#else
    // Non-ARM platforms: timestamp not implemented
    // Platform-specific timestamp implementation required
    return 0;
#endif
}

void rtt_trace_init(uint8_t trace_channel)
{
    bool expected = false;
    if (!trace_initialized.compare_exchange_strong(expected, true))
    {
        return; // Already initialized
    }

    trace_state.channel = trace_channel;
    trace_state.num_registered_tasks = 0;
    dropped_events.store(0, std::memory_order_relaxed);

    // Initialize RTT if not already done
    SEGGER_RTT_Init();

    // Configure a dedicated buffer for the trace channel with adequate size
    SEGGER_RTT_ConfigUpBuffer(trace_channel, "FreeRTOS Trace",
                              rtt_trace_buffer, RTT_TRACE_BUFFER_SIZE,
                              SEGGER_RTT_MODE_NO_BLOCK_SKIP);

    // Send a header marker to identify trace stream
    constexpr char header[] = "RTT_TRACE_V2\n";
    SEGGER_RTT_Write(trace_channel, header, sizeof(header) - 1);
}

void rtt_trace_start(void)
{
    if (trace_initialized.load(std::memory_order_acquire))
    {
        trace_enabled.store(true, std::memory_order_release);

        // Send start marker
        constexpr char start_msg[] = "TRACE_START\n";
        SEGGER_RTT_Write(trace_state.channel, start_msg, sizeof(start_msg) - 1);

        // Send task registry
        rtt_trace_send_task_registry();
    }
}

void rtt_trace_stop(void)
{
    if (trace_enabled.exchange(false, std::memory_order_acq_rel))
    {
        // Send stop marker
        constexpr char stop_msg[] = "TRACE_STOP\n";
        SEGGER_RTT_Write(trace_state.channel, stop_msg, sizeof(stop_msg) - 1);
    }
}

int rtt_trace_is_enabled(void)
{
    return trace_enabled.load(std::memory_order_acquire);
}

uint32_t rtt_trace_get_dropped_event_count(void)
{
    return dropped_events.load(std::memory_order_relaxed);
}

void rtt_trace_record_event(TraceEventType event_type, uint32_t handle, uint32_t data)
{
    if (!rtt_trace_is_enabled())
    {
        return;
    }

    TraceEvent event{};
    event.magic[0] = 'R';
    event.magic[1] = 'T';
    event.version = RTT_TRACE_PROTOCOL_VERSION;
    event.event_type = std::to_underlying(event_type);
    event.dropped_events = dropped_events.exchange(0, std::memory_order_relaxed);
    event.timestamp = rtt_trace_get_timestamp();
    event.handle = handle;
    event.data = data;
    event.crc32 = calculate_crc32(reinterpret_cast<const uint8_t*>(&event), offsetof(TraceEvent, crc32));

    const auto written = SEGGER_RTT_Write(trace_state.channel, &event, sizeof(event));
    if (written != sizeof(event))
    {
        dropped_events.fetch_add(event.dropped_events + 1U, std::memory_order_relaxed);
    }
}

void rtt_trace_register_task(uint32_t handle, const char* name, size_t name_len)
{
    if (!trace_initialized.load(std::memory_order_acquire) || trace_state.num_registered_tasks >= MAX_REGISTERED_TASKS)
    {
        return;
    }

    TaskRegistryEntry* entry = &trace_state.task_registry[trace_state.num_registered_tasks];
    entry->handle = handle;

    const size_t copy_len = name_len < (MAX_TASK_NAME_LEN - 1) ? name_len : (MAX_TASK_NAME_LEN - 1);
    memcpy(entry->name, name, copy_len);
    entry->name[copy_len] = '\0';

    trace_state.num_registered_tasks++;
}

void rtt_trace_send_task_registry(void)
{
    if (!trace_initialized.load(std::memory_order_acquire))
    {
        return;
    }

    // Send task registry header
    constexpr char reg_header[] = "TASK_REGISTRY_START\n";
    SEGGER_RTT_Write(trace_state.channel, reg_header, sizeof(reg_header) - 1);

    // Send each registered task
    for (uint8_t i = 0; i < trace_state.num_registered_tasks; i++)
    {
        TaskRegistryEntry* entry = &trace_state.task_registry[i];

        // Format: "TASK:handle:name\n"
        char buffer[64] = {};
        const int len = std::snprintf(buffer, sizeof(buffer), "TASK:%lu:%s\n",
                                      (unsigned long)entry->handle, entry->name);
        if (len > 0 && static_cast<size_t>(len) < sizeof(buffer))
        {
            SEGGER_RTT_Write(trace_state.channel, buffer, len);
        }
    }

    // Send task registry footer
    constexpr char reg_footer[] = "TASK_REGISTRY_END\n";
    SEGGER_RTT_Write(trace_state.channel, reg_footer, sizeof(reg_footer) - 1);
}

#ifdef __cplusplus

namespace rtt::trace
{
    void FreeRtosTrace::initialize(uint8_t channel) noexcept
    {
        rtt_trace_init(channel);
    }

    void FreeRtosTrace::start() noexcept
    {
        rtt_trace_start();
    }

    void FreeRtosTrace::stop() noexcept
    {
        rtt_trace_stop();
    }

    bool FreeRtosTrace::isEnabled() noexcept
    {
        return rtt_trace_is_enabled() != 0;
    }

    void FreeRtosTrace::recordEvent(TraceEventType type, uint32_t handle, uint32_t data) noexcept
    {
        rtt_trace_record_event(type, handle, data);
    }

    void FreeRtosTrace::registerTask(uint32_t handle, std::string_view name) noexcept
    {
        if (!name.empty())
        {
            // Safe: rtt_trace_register_task uses memcpy with explicit size, doesn't require null-termination
            rtt_trace_register_task(handle, name.data(), name.size());
        }
    }
} // namespace rtt::trace

#endif // __cplusplus

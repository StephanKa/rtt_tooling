#include <rtt_freertos_trace/rtt_freertos_trace.hpp>
#include <SEGGER_RTT.h>
#include <string.h>
#include <cstdio>

constexpr size_t MAX_TASK_NAME_LEN{16};
constexpr size_t MAX_REGISTERED_TASKS{32};
constexpr size_t TRACE_BUFFER_SIZE{512};
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
    uint8_t initialized;
    uint8_t enabled;
    uint8_t channel;
    TaskRegistryEntry task_registry[MAX_REGISTERED_TASKS];
    uint8_t num_registered_tasks;
    uint8_t trace_buffer[TRACE_BUFFER_SIZE];
    uint16_t buffer_pos;
} trace_state = {0, 0, 0, {}, 0, {}, 0};

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
#warning "Timestamp not implemented for this platform. All timestamps will be 0."
    return 0;
#endif
}

void rtt_trace_init(uint8_t trace_channel)
{
    if (trace_state.initialized)
    {
        return; // Already initialized
    }

    trace_state.channel = trace_channel;
    trace_state.enabled = 0;
    trace_state.num_registered_tasks = 0;
    trace_state.buffer_pos = 0;
    trace_state.initialized = 1;

    // Initialize RTT if not already done
    SEGGER_RTT_Init();

    // Configure a dedicated buffer for the trace channel with adequate size
    // Mode: SEGGER_RTT_MODE_NO_BLOCK_SKIP to prevent blocking
    SEGGER_RTT_ConfigUpBuffer(trace_channel, "FreeRTOS Trace",
                              rtt_trace_buffer, RTT_TRACE_BUFFER_SIZE,
                              SEGGER_RTT_MODE_NO_BLOCK_SKIP);

    // Send a header marker to identify trace stream
    constexpr char header[] = "RTT_TRACE_V1\n";
    SEGGER_RTT_Write(trace_channel, header, sizeof(header) - 1);
}

void rtt_trace_start(void)
{
    if (trace_state.initialized)
    {
        trace_state.enabled = 1;

        // Send start marker
        constexpr char start_msg[] = "TRACE_START\n";
        SEGGER_RTT_Write(trace_state.channel, start_msg, sizeof(start_msg) - 1);

        // Send task registry
        rtt_trace_send_task_registry();
    }
}

void rtt_trace_stop(void)
{
    if (trace_state.initialized && trace_state.enabled)
    {
        // Flush any buffered events
        if (trace_state.buffer_pos > 0)
        {
            SEGGER_RTT_Write(trace_state.channel, trace_state.trace_buffer, trace_state.buffer_pos);
            trace_state.buffer_pos = 0;
        }

        // Send stop marker
        constexpr char stop_msg[] = "TRACE_STOP\n";
        SEGGER_RTT_Write(trace_state.channel, stop_msg, sizeof(stop_msg) - 1);

        trace_state.enabled = 0;
    }
}

int rtt_trace_is_enabled(void)
{
    return trace_state.initialized && trace_state.enabled;
}

void rtt_trace_record_event(TraceEventType event_type, uint32_t handle, uint32_t data)
{
    if (!rtt_trace_is_enabled())
    {
        return;
    }

    TraceEvent event;
    event.event_type = static_cast<uint8_t>(event_type);
    event.timestamp = rtt_trace_get_timestamp();
    event.handle = handle;
    event.data = data;

    // Check if we have space in buffer
    if (trace_state.buffer_pos + sizeof(TraceEvent) > TRACE_BUFFER_SIZE)
    {
        // Flush buffer
        SEGGER_RTT_Write(trace_state.channel, trace_state.trace_buffer, trace_state.buffer_pos);
        trace_state.buffer_pos = 0;
    }

    // Add event to buffer
    memcpy(&trace_state.trace_buffer[trace_state.buffer_pos], &event, sizeof(TraceEvent));
    trace_state.buffer_pos += sizeof(TraceEvent);

    // For critical events, flush periodically to avoid buffer overflow
    // Note: event_count is accessed from ISR context, so we protect it
    if (event_type == TRACE_EVENT_TASK_SWITCHED_IN ||
        event_type == TRACE_EVENT_TASK_SWITCHED_OUT ||
        event_type == TRACE_EVENT_ISR_ENTER ||
        event_type == TRACE_EVENT_ISR_EXIT)
    {
        // Simple threshold-based flush without additional state tracking
        // Buffer flush happens automatically when buffer is full
        if (trace_state.buffer_pos >= (TRACE_BUFFER_SIZE / 2))
        {
            SEGGER_RTT_Write(trace_state.channel, trace_state.trace_buffer, trace_state.buffer_pos);
            trace_state.buffer_pos = 0;
        }
    }
}

void rtt_trace_register_task(uint32_t handle, const char* name, size_t name_len)
{
    if (!trace_state.initialized || trace_state.num_registered_tasks >= MAX_REGISTERED_TASKS)
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
    if (!trace_state.initialized)
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

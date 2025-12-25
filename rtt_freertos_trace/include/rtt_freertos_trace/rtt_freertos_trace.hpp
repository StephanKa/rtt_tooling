#pragma once

/**
 * @file rtt_freertos_trace.hpp
 * @brief FreeRTOS tracing via RTT for STM32F205
 *
 * This module provides FreeRTOS tracing capabilities using SEGGER RTT.
 * It captures task switches, interrupts, and timing information without
 * requiring external tools like Tracealyzer.
 *
 * Features:
 * - Task context switches
 * - Interrupt entry/exit
 * - Task creation/deletion
 * - Queue operations
 * - Semaphore/mutex operations
 * - Timing measurements
 *
 * Compatible with J-Link and OpenOCD (ST-Link) for trace data retrieval.
 */

// Include the C declarations from the hooks header
#include <rtt_freertos_trace/rtt_freertos_trace_hooks.h>

#ifdef __cplusplus
#include <string_view>

namespace rtt::trace
{
    /**
     * @brief C++ wrapper for FreeRTOS tracing
     */
    class FreeRtosTrace
    {
    public:
        /**
         * @brief Initialize tracing system
         * @param channel RTT channel for trace output
         */
        static void initialize(uint8_t channel = 1) noexcept;

        /**
         * @brief Start tracing
         */
        static void start() noexcept;

        /**
         * @brief Stop tracing
         */
        static void stop() noexcept;

        /**
         * @brief Check if tracing is active
         * @return true if tracing is enabled
         */
        [[nodiscard]] static bool isEnabled() noexcept;

        /**
         * @brief Record an event
         */
        static void recordEvent(TraceEventType type, uint32_t handle, uint32_t data = 0) noexcept;

        /**
         * @brief Register a task for tracing (using std::string_view)
         */
        static void registerTask(uint32_t handle, std::string_view name) noexcept;
    };
} // namespace rtt::trace

#endif // __cplusplus

/**
 * FreeRTOS Trace Hook Macros
 *
 * NOTE: For use in FreeRTOSConfig.h, include "rtt_freertos_trace_hooks.h" instead
 * of this file to avoid C++ linkage issues. This header (rtt_freertos_trace.hpp)
 * contains C++ code and should only be used in C++ source files.
 *
 * See rtt_freertos_trace_hooks.h for C-only declarations and macros.
 */

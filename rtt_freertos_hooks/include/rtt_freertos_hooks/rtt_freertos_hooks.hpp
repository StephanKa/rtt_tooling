#pragma once

#include <cstdint>
#include <cstddef>

#ifdef __cplusplus
#include <string_view>
extern "C" {
#endif

/**
 * @file rtt_freertos_hooks.hpp
 * @brief FreeRTOS hook functions that use RTT for logging
 *
 * This module provides implementations of FreeRTOS hook functions
 * that output diagnostic information via SEGGER RTT.
 */

/**
 * @brief Hook function called when malloc fails
 *
 * This hook is called when pvPortMalloc fails to allocate memory.
 * It logs the failure via RTT.
 */
void vApplicationMallocFailedHook(void);

/**
 * @brief Hook function called on stack overflow
 *
 * @param pxTask Handle of the task that overflowed its stack
 * @param pcTaskName Name of the task
 */
void vApplicationStackOverflowHook(void* pxTask, char* pcTaskName);

/**
 * @brief Hook function called on each tick
 *
 * Can be used for periodic RTT output or statistics.
 */
void vApplicationTickHook(void);

/**
 * @brief Hook function called when the system is idle
 *
 * Can be used for low-priority background tasks.
 */
void vApplicationIdleHook(void);

/**
 * @brief Hook function called when the daemon task starts
 *
 * @param pvParameters Parameters passed to the daemon task
 */
void vApplicationDaemonTaskStartupHook(void* pvParameters);

#ifdef __cplusplus
}

namespace rtt::freertos {

/**
 * @brief C++ interface for FreeRTOS RTT hooks
 */
class FreeRtosHooks {
public:
    /**
     * @brief Enable or disable verbose hook logging
     * @param enable true to enable, false to disable
     */
    static void setVerbose(bool enable) noexcept;

    /**
     * @brief Check if verbose logging is enabled
     * @return true if enabled, false otherwise
     */
    [[nodiscard]] static bool isVerbose() noexcept;

    /**
     * @brief Log task information via RTT (using std::string_view)
     * @param taskName Name of the task
     * @param message Message to log
     */
    static void logTaskInfo(std::string_view taskName, std::string_view message) noexcept;

    /**
     * @brief Log system statistics via RTT
     */
    static void logSystemStats() noexcept;

private:
    static inline bool m_verbose{false};
};

} // namespace rtt::freertos

#endif // __cplusplus

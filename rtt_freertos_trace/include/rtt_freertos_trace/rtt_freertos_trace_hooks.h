#pragma once

/**
 * @file rtt_freertos_trace_hooks.h
 * @brief C-only header for FreeRTOS trace hooks
 *
 * This header provides ONLY the C function declarations and trace macros,
 * without any C++ code. Use this header in FreeRTOSConfig.h to avoid
 * "template with C linkage" errors.
 *
 * For C++ code, use rtt_freertos_trace.hpp instead.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Trace event types
 */
typedef enum
{
    TRACE_EVENT_TASK_SWITCHED_IN = 0x01,
    TRACE_EVENT_TASK_SWITCHED_OUT = 0x02,
    TRACE_EVENT_TASK_CREATE = 0x03,
    TRACE_EVENT_TASK_DELETE = 0x04,
    TRACE_EVENT_TASK_READY = 0x05,
    TRACE_EVENT_TASK_SUSPENDED = 0x06,
    TRACE_EVENT_TASK_RESUMED = 0x07,
    TRACE_EVENT_ISR_ENTER = 0x10,
    TRACE_EVENT_ISR_EXIT = 0x11,
    TRACE_EVENT_QUEUE_CREATE = 0x20,
    TRACE_EVENT_QUEUE_SEND = 0x21,
    TRACE_EVENT_QUEUE_RECEIVE = 0x22,
    TRACE_EVENT_SEMAPHORE_CREATE = 0x30,
    TRACE_EVENT_SEMAPHORE_GIVE = 0x31,
    TRACE_EVENT_SEMAPHORE_TAKE = 0x32,
    TRACE_EVENT_MUTEX_CREATE = 0x40,
    TRACE_EVENT_MUTEX_GIVE = 0x41,
    TRACE_EVENT_MUTEX_TAKE = 0x42,
    TRACE_EVENT_TIMER_CREATE = 0x50,
    TRACE_EVENT_TIMER_START = 0x51,
    TRACE_EVENT_TIMER_STOP = 0x52,
    TRACE_EVENT_MALLOC = 0x60,
    TRACE_EVENT_FREE = 0x61,
} TraceEventType;

/**
 * @brief Trace event structure
 *
 * Compact binary format for efficient RTT transmission:
 * - Event type (1 byte)
 * - Timestamp (4 bytes) - system tick count
 * - Task/Object handle (4 bytes)
 * - Additional data (4 bytes) - context-dependent
 */
typedef struct __attribute__((packed))
{
    uint8_t event_type;
    uint32_t timestamp;
    uint32_t handle;
    uint32_t data;
} TraceEvent;

/**
 * @brief Initialize the FreeRTOS trace system
 *
 * @param trace_channel RTT channel to use for trace output (default: 1)
 * @return 0 on success, negative on error
 */
void rtt_trace_init(uint8_t trace_channel);

/**
 * @brief Start tracing
 */
void rtt_trace_start(void);

/**
 * @brief Stop tracing
 */
void rtt_trace_stop(void);

/**
 * @brief Check if tracing is enabled
 * @return 1 if enabled, 0 otherwise
 */
int rtt_trace_is_enabled(void);

/**
 * @brief Record a trace event
 *
 * @param event_type Type of event
 * @param handle Task or object handle
 * @param data Additional event-specific data
 */
void rtt_trace_record_event(TraceEventType event_type, uint32_t handle, uint32_t data);

/**
 * @brief Get current timestamp for tracing (in ticks)
 * @return Current tick count
 */
uint32_t rtt_trace_get_timestamp(void);

/**
 * @brief Register a task name for better trace readability
 *
 * @param handle Task handle
 * @param name Task name (will be copied)
 * @param name_len Length of name
 */
void rtt_trace_register_task(uint32_t handle, const char* name, size_t name_len);

/**
 * @brief Send task registry to RTT for Python parser
 */
void rtt_trace_send_task_registry(void);

#ifdef __cplusplus
}
#endif

/**
 * FreeRTOS Trace Hook Macros
 *
 * These macros are automatically available when this header is included.
 * They hook into FreeRTOS trace points to capture events via RTT.
 *
 * Usage in FreeRTOSConfig.h:
 *
 * ```c
 * #include "rtt_freertos_trace/rtt_freertos_trace_hooks.h"
 * #define configUSE_TRACE_FACILITY 1
 * ```
 */

#ifndef traceTASK_SWITCHED_IN
#define traceTASK_SWITCHED_IN() \
    rtt_trace_record_event(TRACE_EVENT_TASK_SWITCHED_IN, (uint32_t)pxCurrentTCB, 0)
#endif

#ifndef traceTASK_SWITCHED_OUT
#define traceTASK_SWITCHED_OUT() \
    rtt_trace_record_event(TRACE_EVENT_TASK_SWITCHED_OUT, (uint32_t)pxCurrentTCB, 0)
#endif

#ifndef traceTASK_CREATE
#define traceTASK_CREATE(pxNewTCB) \
    rtt_trace_record_event(TRACE_EVENT_TASK_CREATE, (uint32_t)pxNewTCB, 0)
#endif

#ifndef traceTASK_DELETE
#define traceTASK_DELETE(pxTaskToDelete) \
    rtt_trace_record_event(TRACE_EVENT_TASK_DELETE, (uint32_t)pxTaskToDelete, 0)
#endif

#ifndef traceMOVED_TASK_TO_READY_STATE
#define traceMOVED_TASK_TO_READY_STATE(pxTCB) \
    rtt_trace_record_event(TRACE_EVENT_TASK_READY, (uint32_t)pxTCB, 0)
#endif

#ifndef traceTASK_SUSPEND
#define traceTASK_SUSPEND(pxTaskToSuspend) \
    rtt_trace_record_event(TRACE_EVENT_TASK_SUSPENDED, (uint32_t)pxTaskToSuspend, 0)
#endif

#ifndef traceTASK_RESUME
#define traceTASK_RESUME(pxTaskToResume) \
    rtt_trace_record_event(TRACE_EVENT_TASK_RESUMED, (uint32_t)pxTaskToResume, 0)
#endif

#ifndef traceTASK_RESUME_FROM_ISR
#define traceTASK_RESUME_FROM_ISR(pxTaskToResume) \
    rtt_trace_record_event(TRACE_EVENT_TASK_RESUMED, (uint32_t)pxTaskToResume, 1)
#endif

#ifndef traceISR_ENTER
#define traceISR_ENTER() \
    rtt_trace_record_event(TRACE_EVENT_ISR_ENTER, 0, 0)
#endif

#ifndef traceISR_EXIT
#define traceISR_EXIT() \
    rtt_trace_record_event(TRACE_EVENT_ISR_EXIT, 0, 0)
#endif

#ifndef traceQUEUE_CREATE
#define traceQUEUE_CREATE(pxNewQueue) \
    rtt_trace_record_event(TRACE_EVENT_QUEUE_CREATE, (uint32_t)pxNewQueue, 0)
#endif

#ifndef traceQUEUE_SEND
#define traceQUEUE_SEND(pxQueue) \
    rtt_trace_record_event(TRACE_EVENT_QUEUE_SEND, (uint32_t)pxQueue, 0)
#endif

#ifndef traceQUEUE_RECEIVE
#define traceQUEUE_RECEIVE(pxQueue) \
    rtt_trace_record_event(TRACE_EVENT_QUEUE_RECEIVE, (uint32_t)pxQueue, 0)
#endif

#ifndef traceMALLOC
#define traceMALLOC(pvAddress, uiSize) \
    rtt_trace_record_event(TRACE_EVENT_MALLOC, (uint32_t)pvAddress, (uint32_t)uiSize)
#endif

#ifndef traceFREE
#define traceFREE(pvAddress, uiSize) \
    rtt_trace_record_event(TRACE_EVENT_FREE, (uint32_t)pvAddress, (uint32_t)uiSize)
#endif

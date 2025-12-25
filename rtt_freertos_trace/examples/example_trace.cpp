/**
 * @file example_trace.cpp
 * @brief Example FreeRTOS application with RTT tracing
 *
 * This example demonstrates how to use the RTT FreeRTOS trace library
 * to capture task switches, interrupts, and timing information.
 *
 * Hardware: STM32F205
 *
 * This example creates multiple tasks and demonstrates:
 * - Task switching
 * - Queue operations
 * - Periodic interrupts
 * - Trace data capture
 */

#include <rtt_freertos_trace/rtt_freertos_trace.hpp>
#include <rtt_logger/rtt_logger.hpp>

// FreeRTOS headers (would be included in real application)
// #include "FreeRTOS.h"
// #include "task.h"
// #include "queue.h"
// #include "semphr.h"

// Example task handles (would be actual handles in real application)
static void* ledTaskHandle = nullptr;
static void* uartTaskHandle = nullptr;
static void* sensorTaskHandle = nullptr;
static void* queueHandle = nullptr;

/**
 * @brief LED blink task
 */
void ledTask([[maybe_unused]]void* params)
{
    while (1)
    {
        // Toggle LED
        // HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

        // Simulate work
        // vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief UART communication task
 */
void uartTask([[maybe_unused]]void* params)
{
    while (1)
    {
        // Process UART data
        // char buffer[32];
        // int len = UART_Receive(buffer, sizeof(buffer));

        // Simulate work
        // vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief Sensor reading task
 */
void sensorTask([[maybe_unused]]void* params)
{
    while (1)
    {
        // Read sensor
        // uint32_t sensor_value = ADC_Read();

        // Send to queue
        // xQueueSend(queueHandle, &sensor_value, portMAX_DELAY);

        // Simulate work
        // vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Initialize RTT tracing
 */
void initializeTracing()
{
    // Initialize RTT logger (channel 0)
    rtt::Logger::initialize();
    auto& logger = rtt::getLogger();
    logger.setMinLevel(rtt::LogLevel::Info);
    logger.info("RTT FreeRTOS Trace Example");

    // Initialize trace system (channel 1)
    rtt_trace_init(1);
    logger.info("Trace system initializing");

    // Note: Tasks should be registered AFTER they are created
    // This is just for demonstration
    logger.info("Trace ready - will start after scheduler");
}

/**
 * @brief Create FreeRTOS tasks
 */
void createTasks()
{
    auto& logger = rtt::getLogger();

    // Create queue for sensor data
    // queueHandle = xQueueCreate(10, sizeof(uint32_t));
    logger.info("Created sensor queue");

    // Create tasks
    // xTaskCreate(ledTask, "LED", 128, nullptr, 1, (TaskHandle_t*)&ledTaskHandle);
    logger.info("Created LED task");

    // xTaskCreate(uartTask, "UART", 256, nullptr, 2, (TaskHandle_t*)&uartTaskHandle);
    logger.info("Created UART task");

    // xTaskCreate(sensorTask, "Sensor", 256, nullptr, 2, (TaskHandle_t*)&sensorTaskHandle);
    logger.info("Created Sensor task");

    // Register tasks with trace system for readable output
    rtt_trace_register_task((uint32_t)ledTaskHandle, "LED", 3);
    rtt_trace_register_task((uint32_t)uartTaskHandle, "UART", 4);
    rtt_trace_register_task((uint32_t)sensorTaskHandle, "Sensor", 6);

    logger.info("Tasks registered with trace system");
}

/**
 * @brief Main application entry point
 */
int main()
{
    // Initialize hardware (in real application)
    // SystemClock_Config();
    // HAL_Init();
    // MX_GPIO_Init();
    // MX_USART1_UART_Init();

    // Initialize tracing
    initializeTracing();

    // Create tasks
    createTasks();

    // Start tracing
    rtt_trace_start();
    rtt::getLogger().info("Trace started - capturing events...");

    // Start FreeRTOS scheduler
    // vTaskStartScheduler();

    // Should never reach here
    while (1)
    {
        // Error: scheduler failed to start
    }

    return 0;
}

/**
 * Example FreeRTOSConfig.h configuration:
 *
 * // Enable trace facility
 * #define configUSE_TRACE_FACILITY                1
 *
 * // Include trace hooks
 * #include "rtt_freertos_trace/rtt_freertos_trace.hpp"
 *
 * // Hook configurations
 * #define INCLUDE_xTaskGetIdleTaskHandle          1
 * #define INCLUDE_pxTaskGetStackStart             1
 *
 * // Optional: Enable stack overflow checking
 * #define configCHECK_FOR_STACK_OVERFLOW          2
 */

/**
 * Example usage with Python scripts:
 *
 * 1. Build and flash the application to STM32F205
 *
 * 2. Capture trace data:
 *    # Using J-Link:
 *    python3 scripts/rtt_trace_reader.py -d STM32F205RB -p jlink -c 1 -o trace.bin
 *
 *    # Using OpenOCD (ST-Link):
 *    openocd -f interface/stlink.cfg -f target/stm32f2x.cfg &
 *    python3 scripts/rtt_trace_reader.py -d stm32f2x -p openocd -c 1 -o trace.bin
 *
 * 3. Analyze trace data:
 *    # Basic statistics:
 *    python3 scripts/rtt_trace_analyzer.py trace.bin --stats
 *
 *    # Task runtime analysis:
 *    python3 scripts/rtt_trace_analyzer.py trace.bin --task-runtime
 *
 *    # Interrupt analysis:
 *    python3 scripts/rtt_trace_analyzer.py trace.bin --interrupts
 *
 *    # Event timeline:
 *    python3 scripts/rtt_trace_analyzer.py trace.bin --timeline
 *
 *    # Export to JSON for custom analysis:
 *    python3 scripts/rtt_trace_analyzer.py trace.bin --export-json trace.json
 */

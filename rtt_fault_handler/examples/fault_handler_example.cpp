/**
 * @file fault_handler_example.cpp
 * @brief Example demonstrating the RTT fault handler functionality
 *
 * This example shows how to initialize and use the fault handler library.
 * The fault handlers will automatically catch hardware faults and send
 * detailed information via RTT including:
 * - Fault type (HardFault, MemManage, BusFault, UsageFault)
 * - CPU registers at the time of fault
 * - Fault status registers (CFSR, HFSR, etc.)
 * - Stack trace
 */

#include "rtt_fault_handler/rtt_fault_handler.hpp"

/**
 * @brief Trigger a divide by zero fault (if enabled in SCB)
 *
 * Note: This only causes a UsageFault if DIV_0_TRP is set in CCR
 */
void triggerDivideByZero()
{
    constexpr int x = 1;
    constexpr int y = 0;
    [[maybe_unused]] const int z = x / y; // Divide by zero
}

/**
 * @brief Trigger an undefined instruction fault
 *
 * This will cause a UsageFault
 */
void triggerUndefinedInstruction()
{
#if defined(__arm__) || defined(__thumb__) || defined(__ARM_ARCH)
    // Execute an undefined instruction (ARM only)
    __asm volatile(".word 0xFFFFFFFF");
#else
    // Placeholder for non-ARM builds
    SEGGER_RTT_WriteString(0, "triggerUndefinedInstruction: Not supported on non-ARM platform\n");
#endif
}

/**
 * @brief Trigger a null pointer dereference
 *
 * This will cause a MemManage fault if MPU is enabled,
 * or a BusFault/HardFault otherwise
 */
void triggerNullPointerDereference()
{
    const uint32_t* nullPtr = nullptr;
    [[maybe_unused]] const uint32_t value = *nullPtr; // Null pointer dereference
}

/**
 * @brief Trigger an unaligned access fault
 *
 * This will cause a UsageFault if UNALIGN_TRP is set in CCR
 */
void triggerUnalignedAccess()
{
    volatile uint8_t buffer[8];
    // Force an unaligned 32-bit access
    volatile uint32_t* unalignedPtr = reinterpret_cast<volatile uint32_t*>(&buffer[1]);
    [[maybe_unused]]const uint32_t value = *unalignedPtr;
}

int main()
{
    // Initialize the fault handler with default configuration
    // This sets up RTT channel 0, max stack depth of 16, and verbose mode
    rtt::fault::FaultHandler::initialize();

    // Alternative: Initialize with custom configuration
    /*
    rtt::fault::FaultHandlerConfig config;
    config.rttChannel = 0;           // Use RTT channel 0
    config.maxStackDepth = 32;       // Show up to 32 stack frames
    config.enableVerbose = true;     // Enable verbose fault decoding

    // Optional: Enable stack boundary checking (for STM32F205 example)
    config.checkStackBounds = true;
    config.stackStart = 0x20000000;  // Start of RAM
    config.stackEnd = 0x20020000;    // End of RAM (128KB)

    rtt::fault::FaultHandler::initialize(config);
    */

    SEGGER_RTT_WriteString(0, "\n=== RTT Fault Handler Example ===\n");
    SEGGER_RTT_WriteString(0, "Fault handlers are now installed.\n");
    SEGGER_RTT_WriteString(0, "Any hardware fault will be caught and reported via RTT.\n\n");

    // Normal operation - no faults
    SEGGER_RTT_WriteString(0, "Running normal operations...\n");

    int counter = 0; // Removed volatile to avoid C++20 deprecation warning
    for (int i = 0; i < 10; ++i)
    {
        counter++;
    }

    SEGGER_RTT_WriteString(0, "Normal operations complete.\n\n");

    // Uncomment one of the following lines to trigger a specific fault:
    // WARNING: These will cause the system to halt after reporting the fault!

    // triggerNullPointerDereference();    // Most common - triggers MemManage/BusFault/HardFault
    // triggerUndefinedInstruction();      // Triggers UsageFault
    // triggerDivideByZero();              // Triggers UsageFault (if enabled)
    // triggerUnalignedAccess();           // Triggers UsageFault (if enabled)

    SEGGER_RTT_WriteString(0, "Example complete. No faults triggered.\n");
    SEGGER_RTT_WriteString(0, "Uncomment a trigger function to test fault handling.\n");

    // Main loop
    while (true)
    {
#if defined(__arm__) || defined(__thumb__) || defined(__ARM_ARCH)
        // In a real application, this would be your main task loop
        __asm volatile("wfi"); // Wait for interrupt (ARM only)
#else
        // Placeholder for non-ARM builds
        break;
#endif
    }

    return 0;
}

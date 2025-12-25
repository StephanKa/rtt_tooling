#include <rtt_freertos_hooks/rtt_freertos_hooks.hpp>
#include <rtt_logger/rtt_logger.hpp>

/**
 * Example demonstrating FreeRTOS hooks with RTT logging
 * 
 * This example shows how to enable and use FreeRTOS hook functions
 * that output diagnostic information via RTT.
 * 
 * Note: This is a demonstration of the API. In a real FreeRTOS application,
 * these hooks are called automatically by the FreeRTOS kernel.
 */

int main() {
    // Initialize RTT logger (required for hooks to output via RTT)
    rtt::Logger::initialize();
    auto& logger = rtt::getLogger();
    logger.setMinLevel(rtt::LogLevel::Info);
    
    logger.info("===========================================");
    logger.info("  FreeRTOS Hooks Example");
    logger.info("===========================================");
    
    // Enable verbose hook logging
    rtt::freertos::FreeRtosHooks::setVerbose(true);
    
    logger.info("");
    logger.info("FreeRTOS hooks are now configured.");
    logger.info("Verbose logging enabled for all hooks.");
    logger.info("");
    logger.info("Available hooks (called automatically by FreeRTOS):");
    logger.info("  - vApplicationMallocFailedHook()");
    logger.info("  - vApplicationStackOverflowHook()");
    logger.info("  - vApplicationTickHook()");
    logger.info("  - vApplicationIdleHook()");
    logger.info("  - vApplicationDaemonTaskStartupHook()");
    logger.info("");
    
    // Example: Demonstrate what happens when malloc fails
    logger.info("Simulating malloc failure...");
    vApplicationMallocFailedHook();
    
    // Example: Demonstrate what happens with stack overflow
    logger.info("");
    logger.info("Simulating stack overflow...");
    char taskName[] = "ExampleTask";
    void* taskHandle = (void*)0xDEADBEEF;  // Dummy handle for demonstration
    vApplicationStackOverflowHook(taskHandle, taskName);
    
    // Example: Demonstrate tick hook (normally called every FreeRTOS tick)
    logger.info("");
    logger.info("The tick hook is called on every FreeRTOS tick:");
    logger.info("Calling tick hook 5 times...");
    for (int i = 0; i < 5; ++i) {
        vApplicationTickHook();
    }
    
    // Example: Demonstrate idle hook (normally called when system is idle)
    logger.info("");
    logger.info("The idle hook is called when FreeRTOS is idle:");
    logger.info("Calling idle hook 3 times...");
    for (int i = 0; i < 3; ++i) {
        vApplicationIdleHook();
    }
    
    // Example: Demonstrate daemon task startup hook
    logger.info("");
    logger.info("The daemon task startup hook is called when timer daemon starts:");
    vApplicationDaemonTaskStartupHook(nullptr);
    
    logger.info("");
    logger.info("===========================================");
    logger.info("  FreeRTOS Hooks Example Completed");
    logger.info("===========================================");
    logger.info("");
    logger.info("Note: In a real FreeRTOS application:");
    logger.info("  1. Enable hooks in FreeRTOSConfig.h");
    logger.info("  2. The hooks are called automatically by FreeRTOS");
    logger.info("  3. Hook output appears via RTT for debugging");
    
    return 0;
}

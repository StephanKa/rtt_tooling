#include <rtt_freertos_hooks/rtt_freertos_hooks.hpp>
#include <rtt_logger/rtt_logger.hpp>
#include <cstring>
#include <tuple>

namespace rtt::freertos {

void FreeRtosHooks::setVerbose(bool enable) noexcept {
    m_verbose = enable;
}

bool FreeRtosHooks::isVerbose() noexcept {
    return m_verbose;
}

void FreeRtosHooks::logTaskInfo(std::string_view taskName, std::string_view message) noexcept {
    auto& logger = rtt::getLogger();

    if (!taskName.empty() && !message.empty()) {
        logger.info("[Task: ");
        std::ignore = logger.write(taskName.data(), taskName.size());
        std::ignore = logger.write("] ", 2);
        std::ignore = logger.write(message.data(), message.size());
        std::ignore = logger.write("\r\n", 2);
    }
}

void FreeRtosHooks::logSystemStats() noexcept {
    auto& logger = rtt::getLogger();
    logger.info("=== System Statistics ===");
}

} // namespace rtt::freertos

extern "C" {
void vApplicationMallocFailedHook(void) {
    auto& logger = rtt::getLogger();
    logger.critical("FreeRTOS: Malloc failed!");

    // In a real system, you might want to halt here
    while (true) {
        // Trap
    }
}

void vApplicationStackOverflowHook([[maybe_unused]]void* pxTask, char* pcTaskName) {
    auto& logger = rtt::getLogger();
    logger.critical("FreeRTOS: Stack overflow in task: ");

    if (pcTaskName != nullptr) {
        std::ignore = logger.write(pcTaskName, std::strlen(pcTaskName));
    } else {
        std::ignore = logger.write("Unknown", 7);
    }
    std::ignore = logger.write("\r\n", 2);

    // In a real system, you might want to halt here
    while (true) {
        // Trap
    }
}

void vApplicationTickHook(void) {
    // Called on each tick - use sparingly to avoid performance impact
    if (rtt::freertos::FreeRtosHooks::isVerbose()) {
        // Could log periodic statistics here
    }
}

void vApplicationIdleHook(void) {
    // Called when the system is idle
    if (rtt::freertos::FreeRtosHooks::isVerbose()) {
        // Could log idle statistics here
    }
}

void vApplicationDaemonTaskStartupHook([[maybe_unused]]void* pvParameters) {
    auto& logger = rtt::getLogger();
    logger.info("FreeRTOS: Daemon task started");
}

} // extern "C"

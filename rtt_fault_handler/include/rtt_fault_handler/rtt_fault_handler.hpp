#pragma once

#include <cstdint>

// Forward declare SEGGER RTT functions
extern "C" {
int SEGGER_RTT_printf(unsigned int BufferIndex, const char* sFormat, ...);
unsigned int SEGGER_RTT_WriteString(unsigned int BufferIndex, const char* s);
}

namespace rtt::fault
{
    /**
         * @brief Fault types for ARM Cortex-M processors
         */
    enum class FaultType : uint8_t
    {
        HardFault = 0,
        MemManageFault,
        BusFault,
        UsageFault,
        Unknown
    };

    /**
         * @brief Stack frame structure pushed by hardware on exception entry
         *
         * This is the stack frame that the Cortex-M processor automatically
         * pushes to the stack when an exception occurs.
         */
    struct ExceptionStackFrame
    {
        uint32_t r0;
        uint32_t r1;
        uint32_t r2;
        uint32_t r3;
        uint32_t r12;
        uint32_t lr; // Link Register
        uint32_t pc; // Program Counter (return address)
        uint32_t psr; // Program Status Register
    };

    /**
         * @brief Configuration for fault handler
         */
    struct FaultHandlerConfig
    {
        uint8_t rttChannel; // RTT channel to use for output (default: 0)
        uint8_t maxStackDepth; // Maximum stack trace depth (default: 16)
        bool enableVerbose; // Enable verbose output (default: true)
        bool checkStackBounds; // Enable stack boundary checking (default: false for portability)
        uintptr_t stackStart; // Stack start address (if checkStackBounds is true)
        uintptr_t stackEnd; // Stack end address (if checkStackBounds is true)

        constexpr FaultHandlerConfig()
            : rttChannel(0), maxStackDepth(16), enableVerbose(true),
              checkStackBounds(false), stackStart(0), stackEnd(0)
        {
        }
    };

    /**
         * @brief Fault handler for ARM Cortex-M processors
         *
         * This class provides comprehensive fault handling with stack traces
         * sent via RTT. It overrides hardware fault handlers for HardFault,
         * MemManage, BusFault, and UsageFault.
         */
    class FaultHandler
    {
    public:
        /**
             * @brief Initialize the fault handler
             *
             * @param config Configuration for the fault handler
             */
        static void initialize(const FaultHandlerConfig& config = FaultHandlerConfig());

        /**
             * @brief Get the current configuration
             *
             * @return Current fault handler configuration
             */
        static const FaultHandlerConfig& getConfig();

        /**
             * @brief Set verbose mode
             *
             * @param enabled Enable or disable verbose output
             */
        static void setVerbose(bool enabled);

        /**
             * @brief Print fault information to RTT
             *
             * @param type Fault type
             * @param stackFrame Pointer to exception stack frame
             */
        static void printFaultInfo(FaultType type, const ExceptionStackFrame* stackFrame);

        /**
             * @brief Print stack trace to RTT
             *
             * @param sp Stack pointer at the time of fault
             * @param maxDepth Maximum number of stack frames to print
             */
        static void printStackTrace(uint32_t* sp, uint8_t maxDepth);

        /**
             * @brief Print CPU registers from exception stack frame
             *
             * @param stackFrame Pointer to exception stack frame
             */
        static void printRegisters(const ExceptionStackFrame* stackFrame);

        /**
             * @brief Print fault status registers (CFSR, HFSR, DFSR, MMFAR, BFAR)
             */
        static void printFaultStatusRegisters();

        /**
             * @brief Get fault type name as string
             *
             * @param type Fault type
             * @return Fault type name
             */
        static const char* getFaultTypeName(FaultType type);

    private:
        static FaultHandlerConfig s_config;
    };
}

// External C linkage for fault handler functions
extern "C" {
/**
 * @brief HardFault handler (overrides weak default)
 */
void HardFault_Handler(void);

/**
 * @brief MemManage fault handler (overrides weak default)
 */
void MemManage_Handler(void);

/**
 * @brief BusFault handler (overrides weak default)
 */
void BusFault_Handler(void);

/**
 * @brief UsageFault handler (overrides weak default)
 */
void UsageFault_Handler(void);

/**
 * @brief Common fault handler implementation
 *
 * This is called by the assembly fault handlers with the
 * stack frame and fault type information.
 *
 * @param stackFrame Pointer to exception stack frame
 * @param faultType Type of fault that occurred
 */
void fault_handler_c(uint32_t* stackFrame, uint8_t faultType);
}

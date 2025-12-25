#include <rtt_fault_handler/rtt_fault_handler.hpp>
#include <cstdint>

namespace rtt::fault
{
    // Static member initialization
    FaultHandlerConfig FaultHandler::s_config;

    void FaultHandler::initialize(const FaultHandlerConfig& config)
    {
        s_config = config;

        if (s_config.enableVerbose)
        {
            SEGGER_RTT_printf(s_config.rttChannel,
                              "\n[Fault Handler] Initialized (RTT Channel: %u, Max Stack Depth: %u)\n",
                              s_config.rttChannel, s_config.maxStackDepth);
        }
    }

    const FaultHandlerConfig& FaultHandler::getConfig()
    {
        return s_config;
    }

    void FaultHandler::setVerbose(bool enabled)
    {
        s_config.enableVerbose = enabled;
    }

    const char* FaultHandler::getFaultTypeName(FaultType type)
    {
        switch (type)
        {
        case FaultType::HardFault:
            return "HardFault";
        case FaultType::MemManageFault:
            return "MemManage Fault";
        case FaultType::BusFault:
            return "BusFault";
        case FaultType::UsageFault:
            return "UsageFault";
        default:
            return "Unknown Fault";
        }
    }

    void FaultHandler::printFaultInfo(FaultType type, const ExceptionStackFrame* stackFrame)
    {
        SEGGER_RTT_WriteString(s_config.rttChannel, "\n");
        SEGGER_RTT_WriteString(s_config.rttChannel, "=================================\n");
        SEGGER_RTT_WriteString(s_config.rttChannel, "     FAULT EXCEPTION DETECTED    \n");
        SEGGER_RTT_WriteString(s_config.rttChannel, "=================================\n");

        SEGGER_RTT_printf(s_config.rttChannel, "Fault Type: %s\n", getFaultTypeName(type));

        if (stackFrame != nullptr)
        {
            printRegisters(stackFrame);
        }

        printFaultStatusRegisters();
    }

    void FaultHandler::printRegisters(const ExceptionStackFrame* stackFrame)
    {
        SEGGER_RTT_WriteString(s_config.rttChannel, "\n--- CPU Registers ---\n");
        SEGGER_RTT_printf(s_config.rttChannel, "R0  = 0x%08lX\n", stackFrame->r0);
        SEGGER_RTT_printf(s_config.rttChannel, "R1  = 0x%08lX\n", stackFrame->r1);
        SEGGER_RTT_printf(s_config.rttChannel, "R2  = 0x%08lX\n", stackFrame->r2);
        SEGGER_RTT_printf(s_config.rttChannel, "R3  = 0x%08lX\n", stackFrame->r3);
        SEGGER_RTT_printf(s_config.rttChannel, "R12 = 0x%08lX\n", stackFrame->r12);
        SEGGER_RTT_printf(s_config.rttChannel, "LR  = 0x%08lX\n", stackFrame->lr);
        SEGGER_RTT_printf(s_config.rttChannel, "PC  = 0x%08lX\n", stackFrame->pc);
        SEGGER_RTT_printf(s_config.rttChannel, "PSR = 0x%08lX\n", stackFrame->psr);
    }

    void FaultHandler::printFaultStatusRegisters()
    {
        // Fault status register addresses for ARM Cortex-M3/M4
        const volatile uint32_t* SCB_CFSR = reinterpret_cast<volatile uint32_t*>(0xE000ED28);
        // Configurable Fault Status Register
        const volatile uint32_t* SCB_HFSR = reinterpret_cast<volatile uint32_t*>(0xE000ED2C);
        // HardFault Status Register
        const volatile uint32_t* SCB_DFSR = reinterpret_cast<volatile uint32_t*>(0xE000ED30);
        // Debug Fault Status Register
        const volatile uint32_t* SCB_MMFAR = reinterpret_cast<volatile uint32_t*>(0xE000ED34);
        // MemManage Fault Address Register
        const volatile uint32_t* SCB_BFAR = reinterpret_cast<volatile uint32_t*>(0xE000ED38);
        // BusFault Address Register

        SEGGER_RTT_WriteString(s_config.rttChannel, "\n--- Fault Status Registers ---\n");
        SEGGER_RTT_printf(s_config.rttChannel, "CFSR  = 0x%08lX\n", *SCB_CFSR);
        SEGGER_RTT_printf(s_config.rttChannel, "HFSR  = 0x%08lX\n", *SCB_HFSR);
        SEGGER_RTT_printf(s_config.rttChannel, "DFSR  = 0x%08lX\n", *SCB_DFSR);
        SEGGER_RTT_printf(s_config.rttChannel, "MMFAR = 0x%08lX\n", *SCB_MMFAR);
        SEGGER_RTT_printf(s_config.rttChannel, "BFAR  = 0x%08lX\n", *SCB_BFAR);

        // Decode CFSR bits if verbose is enabled
        if (s_config.enableVerbose)
        {
            const uint32_t cfsr = *SCB_CFSR;

            SEGGER_RTT_WriteString(s_config.rttChannel, "\n--- CFSR Decode ---\n");

            // MemManage Fault Status (bits 0-7)
            if (cfsr & 0x01)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  IACCVIOL: Instruction access violation\n");
            }
            if (cfsr & 0x02)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  DACCVIOL: Data access violation\n");
            }
            if (cfsr & 0x08)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  MUNSTKERR: MemManage fault on unstacking\n");
            }
            if (cfsr & 0x10)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  MSTKERR: MemManage fault on stacking\n");
            }
            if (cfsr & 0x20)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel,
                                       "  MLSPERR: MemManage fault during lazy FP state preservation\n");
            }
            if (cfsr & 0x80) { SEGGER_RTT_WriteString(s_config.rttChannel, "  MMARVALID: MMFAR valid\n"); }

            // BusFault Status (bits 8-15)
            if (cfsr & 0x0100)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  IBUSERR: Instruction bus error\n");
            }
            if (cfsr & 0x0200)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  PRECISERR: Precise data bus error\n");
            }
            if (cfsr & 0x0400)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  IMPRECISERR: Imprecise data bus error\n");
            }
            if (cfsr & 0x0800)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  UNSTKERR: BusFault on unstacking\n");
            }
            if (cfsr & 0x1000)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  STKERR: BusFault on stacking\n");
            }
            if (cfsr & 0x2000)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel,
                                       "  LSPERR: BusFault during lazy FP state preservation\n");
            }
            if (cfsr & 0x8000)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  BFARVALID: BFAR valid\n");
            }

            // UsageFault Status (bits 16-31)
            if (cfsr & 0x00010000)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  UNDEFINSTR: Undefined instruction\n");
            }
            if (cfsr & 0x00020000)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  INVSTATE: Invalid state\n");
            }
            if (cfsr & 0x00040000)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  INVPC: Invalid PC load\n");
            }
            if (cfsr & 0x00080000)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  NOCP: No coprocessor\n");
            }
            if (cfsr & 0x01000000)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  UNALIGNED: Unaligned access\n");
            }
            if (cfsr & 0x02000000)
            {
                SEGGER_RTT_WriteString(s_config.rttChannel, "  DIVBYZERO: Divide by zero\n");
            }
        }
    }

    void FaultHandler::printStackTrace(uint32_t* sp, uint8_t maxDepth)
    {
        SEGGER_RTT_WriteString(s_config.rttChannel, "\n--- Stack Trace ---\n");
        SEGGER_RTT_printf(s_config.rttChannel, "Stack Pointer: 0x%08lX\n",
                          static_cast<unsigned long>(reinterpret_cast<uintptr_t>(sp)));

        // Print stack contents
        SEGGER_RTT_WriteString(s_config.rttChannel, "\nStack dump (first frames):\n");
        for (uint8_t i = 0; i < maxDepth && i < s_config.maxStackDepth; ++i)
        {
            const uintptr_t addr = reinterpret_cast<uintptr_t>(&sp[i]);

            // Check stack boundaries if enabled
            if (s_config.checkStackBounds)
            {
                if (addr < s_config.stackStart || addr >= s_config.stackEnd)
                {
                    SEGGER_RTT_WriteString(s_config.rttChannel, "  [Stack boundary reached]\n");
                    break;
                }
            }

            SEGGER_RTT_printf(s_config.rttChannel, "  [%02u] 0x%08lX: 0x%08lX\n",
                              i, static_cast<unsigned long>(addr), sp[i]);
        }

        SEGGER_RTT_WriteString(s_config.rttChannel, "\n=================================\n\n");
    }
}

// Fault handler C implementations
extern "C" {
/**
 * @brief Common fault handler that processes all fault types
 *
 * This function is called by the individual fault handlers with
 * the appropriate fault type and stack frame pointer.
 */
void fault_handler_c(uint32_t* stackFrame, uint8_t faultType)
{
    using namespace rtt::fault;

    auto* exceptionFrame = reinterpret_cast<ExceptionStackFrame*>(stackFrame);
    auto type = static_cast<FaultType>(faultType);

    // Print fault information
    FaultHandler::printFaultInfo(type, exceptionFrame);

    // Print stack trace
    FaultHandler::printStackTrace(stackFrame, FaultHandler::getConfig().maxStackDepth);

    // Infinite loop to halt execution
    while (1)
    {
        // Optionally could trigger a system reset here
        __asm volatile("nop");
    }
}

#if defined(__arm__) || defined(__thumb__) || defined(__ARM_ARCH)
// ARM Cortex-M specific fault handlers with inline assembly

/**
 * @brief Naked HardFault handler
 *
 * This handler determines which stack pointer was in use
 * and calls the common C handler with the stack frame.
 */
__attribute__((naked))
void HardFault_Handler(void)
{
    __asm volatile(
        "tst lr, #4\n" // Test bit 2 of LR to determine stack
        "ite eq\n" // If-Then-Else
        "mrseq r0, msp\n" // If bit 2 is 0, use MSP
        "mrsne r0, psp\n" // If bit 2 is 1, use PSP
        "mov r1, #0\n" // Fault type: HardFault
        "b fault_handler_c\n" // Branch to C handler
        ::: "r0", "r1"
    );
}

/**
 * @brief Naked MemManage fault handler
 */
__attribute__((naked))
void MemManage_Handler(void)
{
    __asm volatile(
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq r0, msp\n"
        "mrsne r0, psp\n"
        "mov r1, #1\n" // Fault type: MemManageFault
        "b fault_handler_c\n"
        ::: "r0", "r1"
    );
}

/**
 * @brief Naked BusFault handler
 */
__attribute__((naked))
void BusFault_Handler(void)
{
    __asm volatile(
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq r0, msp\n"
        "mrsne r0, psp\n"
        "mov r1, #2\n" // Fault type: BusFault
        "b fault_handler_c\n"
        ::: "r0", "r1"
    );
}

/**
 * @brief Naked UsageFault handler
 */
__attribute__((naked))
void UsageFault_Handler(void)
{
    __asm volatile(
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq r0, msp\n"
        "mrsne r0, psp\n"
        "mov r1, #3\n" // Fault type: UsageFault
        "b fault_handler_c\n"
        ::: "r0", "r1"
    );
}

#else
// Non-ARM stubs for compilation on other platforms (e.g., for testing the API)

void HardFault_Handler(void)
{
    // Stub for non-ARM builds
    SEGGER_RTT_WriteString(0, "HardFault_Handler stub called (non-ARM build)\n");
}

void MemManage_Handler(void)
{
    // Stub for non-ARM builds
    SEGGER_RTT_WriteString(0, "MemManage_Handler stub called (non-ARM build)\n");
}

void BusFault_Handler(void)
{
    // Stub for non-ARM builds
    SEGGER_RTT_WriteString(0, "BusFault_Handler stub called (non-ARM build)\n");
}

void UsageFault_Handler(void)
{
    // Stub for non-ARM builds
    SEGGER_RTT_WriteString(0, "UsageFault_Handler stub called (non-ARM build)\n");
}

#endif // ARM-specific implementations
} // extern "C"

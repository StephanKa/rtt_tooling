# RTT Fault Handler

ARM Cortex-M hardware fault handler with comprehensive error reporting via SEGGER RTT.

## Features

- **Comprehensive fault handling** - HardFault, MemManage, BusFault, UsageFault
- **Detailed register dump** - R0-R3, R12, LR, PC, PSR
- **Fault status registers** - CFSR, HFSR, DFSR, MMFAR, BFAR
- **Fault decoding** - Human-readable fault cause explanation
- **Stack trace** - Configurable stack dump depth
- **Automatic stack detection** - MSP vs PSP identification
- **RTT output** - Real-time fault information without debugger
- **Configurable verbosity** - Control detail level
- **Cross-platform stubs** - Compiles on non-ARM platforms for testing

## Requirements

- ARM Cortex-M processor (M0, M0+, M3, M4, M7)
- RTT Logger library
- SEGGER RTT library
- CMake 3.20 or later
- ARM GCC toolchain for target builds

## Integration

### Add to CMakeLists.txt

```cmake
target_link_libraries(your_application
    PRIVATE
        rtt_fault_handler
        rtt_logger
)
```

### Initialize in Application

```cpp
#include <rtt_fault_handler/rtt_fault_handler.hpp>
#include <rtt_logger/rtt_logger.hpp>

int main() {
    // Initialize RTT logger
    rtt::Logger::initialize();
    
    // Initialize fault handler with default configuration
    rtt::fault::FaultHandler::initialize();
    
    // Your application code...
    
    return 0;
}
```

## API Reference

### FaultHandlerConfig Structure

```cpp
struct FaultHandlerConfig {
    unsigned int rttChannel = 0;      // RTT channel for fault output
    size_t maxStackDepth = 16;        // Maximum stack frames to dump
    bool enableVerbose = true;        // Enable verbose fault decoding
};
```

### FaultHandler Class

```cpp
class FaultHandler {
public:
    // Initialize with default configuration
    static void initialize();
    
    // Initialize with custom configuration
    static void initialize(const FaultHandlerConfig& config);
};
```

## Usage Examples

### Basic Initialization

```cpp
#include <rtt_fault_handler/rtt_fault_handler.hpp>

int main() {
    rtt::Logger::initialize();
    
    // Initialize with defaults
    rtt::fault::FaultHandler::initialize();
    
    // Application runs...
    // Any fault will be caught and reported via RTT
}
```

### Custom Configuration

```cpp
#include <rtt_fault_handler/rtt_fault_handler.hpp>

int main() {
    rtt::Logger::initialize();
    
    // Configure fault handler
    rtt::fault::FaultHandlerConfig config;
    config.rttChannel = 0;           // RTT channel 0 (logs)
    config.maxStackDepth = 32;       // Show 32 stack frames
    config.enableVerbose = true;     // Enable detailed fault info
    
    rtt::fault::FaultHandler::initialize(config);
    
    // Application code...
}
```

### Production Configuration

```cpp
// Minimal fault info for production
rtt::fault::FaultHandlerConfig config;
config.rttChannel = 0;
config.maxStackDepth = 8;          // Reduce stack depth
config.enableVerbose = false;      // Disable verbose decoding
rtt::fault::FaultHandler::initialize(config);
```

## Fault Output Example

When a fault occurs, the handler outputs detailed information via RTT:

```
************************************
*     FAULT EXCEPTION DETECTED     *
************************************
Fault Type: HardFault

--- CPU Registers ---
R0  = 0x00000000
R1  = 0x20001234
R2  = 0x00000042
R3  = 0x00000000
R12 = 0x00000000
LR  = 0x080012AB
PC  = 0x08001234
PSR = 0x21000000

--- Fault Status Registers ---
CFSR  = 0x00000200
HFSR  = 0x40000000
DFSR  = 0x00000000
MMFAR = 0x00000000
BFAR  = 0x00000000

--- CFSR Decode ---
  PRECISERR: Precise data bus error

--- Stack Trace ---
Stack Pointer: 0x20000F80

Stack dump (first 16 frames):
  [00] 0x20000F80: 0x00000000
  [01] 0x20000F84: 0x20001234
  [02] 0x20000F88: 0x080012AB
  [03] 0x20000F8C: 0x00000042
  ...
************************************
```

## Handled Fault Types

### HardFault
- **Cause**: Escalated fault or direct fault
- **Common reasons**: Bus errors, memory access violations, undefined instructions
- **Action**: Check HFSR and CFSR for details

### MemManage Fault
- **Cause**: MPU violation or memory access error
- **Common reasons**: Stack overflow, invalid memory access
- **Action**: Check MMFAR for fault address

### BusFault
- **Cause**: Bus error during instruction fetch or data access
- **Common reasons**: Invalid memory access, peripheral access errors
- **Action**: Check BFAR for fault address (if BFARVALID set)

### UsageFault
- **Cause**: Instruction execution error
- **Common reasons**: Undefined instruction, divide by zero, unaligned access
- **Action**: Check CFSR usage fault bits

## Fault Status Register Decoding

The handler decodes the following registers:

### CFSR (Configurable Fault Status Register)
- **Usage Fault** (bits 16-31)
  - DIVBYZERO: Division by zero
  - UNALIGNED: Unaligned memory access
  - NOCP: No coprocessor
  - INVPC: Invalid PC load
  - INVSTATE: Invalid state
  - UNDEFINSTR: Undefined instruction
  
- **Bus Fault** (bits 8-15)
  - BFARVALID: BFAR contains valid address
  - LSPERR: Lazy FP state preservation error
  - STKERR: Stack push error
  - UNSTKERR: Stack pop error
  - IMPRECISERR: Imprecise data bus error
  - PRECISERR: Precise data bus error
  - IBUSERR: Instruction bus error
  
- **MemManage Fault** (bits 0-7)
  - MMARVALID: MMFAR contains valid address
  - MLSPERR: Lazy FP state preservation error
  - MSTKERR: Stack push error
  - MUNSTKERR: Stack pop error
  - DACCVIOL: Data access violation
  - IACCVIOL: Instruction access violation

### HFSR (HardFault Status Register)
- FORCED: Forced HardFault
- VECTTBL: Vector table read fault
- DEBUGEVT: Debug event fault

## Examples

See the [examples](examples/) directory for complete examples:

- `fault_handler_example.cpp` - Demonstration of fault handler functionality

### Building Examples

```bash
# ARM build (fault handler requires ARM target)
cmake --preset arm-stm32f205
cmake --build --preset arm-stm32f205
# Flash to device and trigger fault to see output
```

## Common Fault Scenarios

### Null Pointer Dereference

```cpp
int* ptr = nullptr;
*ptr = 42;  // Triggers MemManage or HardFault
```

**Output:** DACCVIOL (Data access violation)

### Stack Overflow

```cpp
void recursiveFunction() {
    char buffer[1024];
    recursiveFunction();  // Eventually overflows stack
}
```

**Output:** MSTKERR (Stack push error)

### Invalid Memory Access

```cpp
volatile uint32_t* invalid = (uint32_t*)0xFFFFFFFF;
uint32_t value = *invalid;  // Triggers BusFault
```

**Output:** PRECISERR (Precise data bus error)

### Undefined Instruction

```cpp
void (*invalidFunc)(void) = (void(*)(void))0xFFFFFFFF;
invalidFunc();  // Triggers UsageFault
```

**Output:** UNDEFINSTR (Undefined instruction)

### Division by Zero

```cpp
// Enable UsageFault for divide-by-zero (in startup code)
// SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;

int a = 10;
int b = 0;
int result = a / b;  // Triggers UsageFault
```

**Output:** DIVBYZERO (Division by zero)

## Best Practices

1. **Initialize early**: Call before main application code
2. **Enable fault handlers**: Ensure UsageFault, BusFault, MemManage are enabled
3. **Sufficient stack**: Ensure stack has space for fault handler
4. **RTT buffer**: Ensure RTT buffer is large enough for fault output
5. **Monitor RTT**: Always monitor RTT output during development
6. **Log analysis**: Save fault logs for post-mortem analysis
7. **Production builds**: Consider reducing verbosity for production

## Enabling Fault Handlers

Add to your startup code or system initialization:

```cpp
// Enable UsageFault, BusFault, MemManage
SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk |
              SCB_SHCSR_BUSFAULTENA_Msk |
              SCB_SHCSR_MEMFAULTENA_Msk;

// Optional: Enable divide-by-zero trap
SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;

// Optional: Enable unaligned access trap
SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
```

## Troubleshooting

### No fault output
1. Verify RTT is initialized before fault
2. Check RTT buffer hasn't overflowed
3. Ensure debug probe is connected
4. Verify fault handlers are installed (check startup code)

### Incomplete fault output
1. Increase RTT buffer size
2. Reduce maxStackDepth
3. Disable verbose mode
4. Ensure sufficient stack space

### Fault handler crashes
1. Check stack pointer is valid
2. Ensure RTT is initialized
3. Verify configuration is reasonable
4. Check for stack overflow

## Reading Fault Output

### Using J-Link
```bash
python3 scripts/rtt_reader.py --backend jlink --device STM32F205RB --output fault.txt
```

### Using OpenOCD
```bash
# Start OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg

# Read fault output
python3 scripts/rtt_reader.py --backend openocd --host localhost --port 4444 --output fault.txt
```

## Platform Support

- **ARM Cortex-M**: Full support (M0, M0+, M3, M4, M7)
- **Other platforms**: Stub implementation (compiles but doesn't handle faults)

## Performance Impact

- **No fault**: Zero overhead (handlers installed but not called)
- **Fault occurs**: ~1-5ms for output (depends on stack depth and verbosity)
- **Memory**: ~500 bytes flash for handler code
- **Stack**: ~100-200 bytes during fault handling

## See Also

- [Main README](../README.md) - Project overview
- [RTT Logger](../rtt_logger/) - Core logging functionality
- [ARM Cortex-M Documentation](https://developer.arm.com/documentation/ddi0403/latest/)
- [Fault Handling Guide](https://interrupt.memfault.com/blog/cortex-m-fault-debug)

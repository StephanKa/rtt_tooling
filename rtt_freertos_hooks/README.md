# RTT FreeRTOS Hooks

FreeRTOS hook functions that provide system diagnostics via SEGGER RTT for embedded debugging.

## Features

- **Malloc failure detection** - Alerts when heap allocation fails
- **Stack overflow detection** - Reports task stack overflows with task identification
- **Tick hook** - Optional periodic diagnostics on each FreeRTOS tick
- **Idle hook** - Low-priority background diagnostics when system is idle
- **Daemon task startup hook** - Notification when timer daemon starts
- **Configurable verbosity** - Control detail level of hook output
- **Minimal overhead** - Optimized for embedded systems
- **RTT output** - Real-time diagnostics without halting the system

## Requirements

- FreeRTOS
- RTT Logger library
- SEGGER RTT library (automatically fetched by CMake)
- CMake 3.20 or later

## Integration

### 1. Add to CMakeLists.txt

```cmake
target_link_libraries(your_application
    PRIVATE
        rtt_freertos_hooks
        rtt_logger
)
```

### 2. Configure FreeRTOSConfig.h

Enable the hooks you want to use:

```c
// Enable malloc failed hook
#define configUSE_MALLOC_FAILED_HOOK        1

// Enable stack overflow checking (method 1 or 2)
#define configCHECK_FOR_STACK_OVERFLOW      2

// Enable tick hook (optional - adds overhead to each tick)
#define configUSE_TICK_HOOK                 1

// Enable idle hook
#define configUSE_IDLE_HOOK                 1

// Enable daemon task startup hook
#define configUSE_DAEMON_TASK_STARTUP_HOOK  1
```

### 3. Initialize RTT in your application

```cpp
#include <rtt_logger/rtt_logger.hpp>
#include <rtt_freertos_hooks/rtt_freertos_hooks.hpp>

int main() {
    // Initialize RTT logger
    rtt::Logger::initialize();
    
    // Optional: Enable verbose hook logging
    rtt::freertos::FreeRtosHooks::setVerbose(true);
    
    // Create and start FreeRTOS tasks...
    
    // Start FreeRTOS scheduler
    vTaskStartScheduler();
}
```

## API Reference

### C++ Interface

```cpp
namespace rtt::freertos {
    class FreeRtosHooks {
    public:
        // Enable/disable verbose logging
        static void setVerbose(bool verbose);
        
        // Get current verbose setting
        static bool isVerbose();
    };
}
```

### Hook Functions (Called by FreeRTOS)

```c
// Called when malloc fails
void vApplicationMallocFailedHook(void);

// Called on stack overflow
void vApplicationStackOverflowHook(void* pxTask, char* pcTaskName);

// Called on each FreeRTOS tick
void vApplicationTickHook(void);

// Called when system is idle
void vApplicationIdleHook(void);

// Called when timer daemon task starts
void vApplicationDaemonTaskStartupHook(void);
```

## Usage

The hooks are automatically called by FreeRTOS when the corresponding events occur. You don't need to call them manually in your application code.

### Enable Verbose Logging

```cpp
// Enable detailed logging from all hooks
rtt::freertos::FreeRtosHooks::setVerbose(true);

// Disable verbose logging (errors still reported)
rtt::freertos::FreeRtosHooks::setVerbose(false);
```

## Hook Descriptions

### Malloc Failed Hook

**When called**: `pvPortMalloc()` fails to allocate memory

**Output example**:
```
[ERROR] FreeRTOS: Malloc failed!
```

**Action**: Investigate memory usage, increase heap size, or reduce allocations

### Stack Overflow Hook

**When called**: Task stack overflow is detected (configCHECK_FOR_STACK_OVERFLOW)

**Output example**:
```
[ERROR] FreeRTOS: Stack overflow in task: MyTask
```

**Action**: Increase stack size for the reported task

### Tick Hook

**When called**: Every FreeRTOS tick (if enabled)

**Note**: Only enable if needed - adds overhead to every tick

**Verbose output**: Tick counter and diagnostics

**Action**: Use for periodic monitoring or watchdog kicks

### Idle Hook

**When called**: When no tasks are ready to run

**Verbose output**: Idle state information

**Action**: Use for low-priority background tasks or power saving

### Daemon Task Startup Hook

**When called**: When FreeRTOS timer daemon task starts

**Verbose output**: Daemon task startup notification

**Action**: Useful for initialization that requires timer service

## Examples

See the [examples](examples/) directory for complete examples:

- `hooks_example.cpp` - Demonstration of all hook functions

### Building Examples

```bash
# Native build (demonstration only - requires FreeRTOS)
cmake --preset default
cmake --build --preset default

# ARM build for real hardware
cmake --preset arm-stm32f205
cmake --build --preset arm-stm32f205
# Flash and view output via RTT
```

## Best Practices

1. **Always enable stack overflow checking** in development builds
2. **Enable malloc failed hook** to catch memory allocation issues early
3. **Use verbose mode** during development, disable in production
4. **Minimize tick hook usage** - it runs on every tick (adds overhead)
5. **Monitor RTT output** during development to catch issues early
6. **Increase stack sizes** if you see stack overflow messages

## Performance Impact

- **Malloc/Stack hooks**: Negligible (only called on errors)
- **Idle hook**: Negligible (only when system idle)
- **Daemon startup**: One-time (during initialization)
- **Tick hook**: Runs every tick - minimize work done here
  - Typical overhead: <1µs per tick with verbose=false
  - Verbose mode: ~5-10µs per tick (for logging)

## Troubleshooting

### Hook not being called
1. Verify hook is enabled in `FreeRTOSConfig.h`
2. Check that FreeRTOS is running (scheduler started)
3. For stack overflow: Ensure `configCHECK_FOR_STACK_OVERFLOW` is 1 or 2

### No RTT output from hooks
1. Ensure `rtt::Logger::initialize()` is called before FreeRTOS starts
2. Check RTT viewer is connected to channel 0
3. Verify debug probe connection

### System becomes unresponsive
1. Check tick hook isn't doing too much work
2. Verify stack sizes are sufficient
3. Monitor heap usage

## Stack Overflow Detection Methods

FreeRTOS provides two methods (set via `configCHECK_FOR_STACK_OVERFLOW`):

### Method 1 (Value = 1)
- Checks if stack pointer is within valid range
- Fast but may miss some overflows
- Recommended for most applications

### Method 2 (Value = 2)
- Method 1 plus checks for stack pattern corruption
- More thorough but slightly slower
- Recommended for critical applications

## Reading RTT Output

### Using J-Link
```bash
python3 scripts/rtt_reader.py --backend jlink --device STM32F205RB
```

### Using OpenOCD
```bash
# Start OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg

# In another terminal
python3 scripts/rtt_reader.py --backend openocd --host localhost --port 4444
```

## See Also

- [Main README](../README.md) - Project overview
- [RTT Logger](../rtt_logger/) - Core logging functionality
- [RTT FreeRTOS Trace](../rtt_freertos_trace/) - Advanced FreeRTOS tracing
- [FreeRTOS Documentation](https://www.freertos.org/a00016.html) - Hook functions reference

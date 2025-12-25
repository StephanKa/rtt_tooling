# FreeRTOS Tracing via RTT

This module provides comprehensive FreeRTOS tracing capabilities using SEGGER RTT for the STM32F205 microcontroller. It allows you to trace task switches, interrupts, and timing information without requiring external commercial tools like Tracealyzer.

## Features

- **Task Tracing**: Monitor task switches, creation, deletion, and states
- **Interrupt Tracing**: Track interrupt entry/exit and timing
- **Queue/Semaphore/Mutex Tracing**: Monitor synchronization primitives
- **High-Resolution Timing**: Uses ARM DWT cycle counter for precise timestamps
- **Minimal Overhead**: Efficient binary format for low impact on system performance
- **Open Source**: No proprietary tools required
- **Dual Probe Support**: Compatible with both J-Link and OpenOCD (ST-Link)

## Hardware Requirements

- STM32F205 microcontroller (or compatible STM32F2 series)
- Debug probe:
  - SEGGER J-Link (any variant), OR
  - ST-Link V2/V3 (via OpenOCD)

## Software Requirements

### Embedded (STM32)
- FreeRTOS
- ARM GCC toolchain
- SEGGER RTT library (included in rtt_logger)

### Host PC
- Python 3.6 or later
- For J-Link:
  - `pylink-square` package: `pip install pylink-square`
- For OpenOCD:
  - OpenOCD 0.11.0 or later
  - Python `telnetlib` (standard library)

## Integration Guide

### 1. Include the Trace Library

Add to your CMakeLists.txt:
```cmake
target_link_libraries(your_application
    PRIVATE
        rtt_freertos_trace
)
```

### 2. Configure FreeRTOS

Add to your `FreeRTOSConfig.h`:

```c
/* Include the C-only trace header (not the .hpp file!) */
#include "rtt_freertos_trace/rtt_freertos_trace_hooks.h"

/* Enable trace facility - macros are now available */
#define configUSE_TRACE_FACILITY                1
```

**Important**: Use `rtt_freertos_trace_hooks.h` (C header) in `FreeRTOSConfig.h`, not the `.hpp` file which contains C++ code and will cause "template with C linkage" errors.

### 3. Initialize Tracing

In your application initialization (order is critical):

```c
#include "rtt_freertos_trace/rtt_freertos_trace.hpp"

int main(void) {
    // Initialize hardware...
    
    // 1. Initialize RTT tracing on channel 1
    rtt_trace_init(1);
    
    // 2. Create your FreeRTOS tasks
    TaskHandle_t task1, task2;
    xTaskCreate(task1_func, "LED_Task", 128, NULL, 1, &task1);
    xTaskCreate(task2_func, "UART_Task", 128, NULL, 2, &task2);
    
    // 3. Register tasks BEFORE starting trace (for readable output)
    rtt_trace_register_task((uint32_t)task1, "LED_Task", 8);
    rtt_trace_register_task((uint32_t)task2, "UART_Task", 9);
    
    // 4. Start tracing (sends task registry to analyzer)
    rtt_trace_start();
    
    // 5. Start FreeRTOS scheduler
    vTaskStartScheduler();
    
    // Should never reach here
    while(1);
}
```

**Important**: Tasks must be registered **before** calling `rtt_trace_start()` because the task registry is sent when tracing starts.

### 4. C++ API (Optional)

```cpp
#include "rtt_freertos_trace/rtt_freertos_trace.hpp"

// Initialize
rtt::trace::FreeRtosTrace::initialize(1);

// Register tasks
rtt::trace::FreeRtosTrace::registerTask((uint32_t)handle, "TaskName");

// Start/stop tracing
rtt::trace::FreeRtosTrace::start();
rtt::trace::FreeRtosTrace::stop();
```

## Capturing Trace Data

### Using J-Link

```bash
# Real-time viewing
python3 scripts/rtt_trace_reader.py -d STM32F205RB -p jlink -c 1

# Save to file for analysis
python3 scripts/rtt_trace_reader.py -d STM32F205RB -p jlink -c 1 -o trace.bin
```

### Using OpenOCD with ST-Link

1. Start OpenOCD in one terminal:
```bash
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg
```

2. In another terminal, run the trace reader:
```bash
# Real-time viewing
python3 scripts/rtt_trace_reader.py -d stm32f2x -p openocd -c 1

# Save to file
python3 scripts/rtt_trace_reader.py -d stm32f2x -p openocd -c 1 -o trace.bin
```

## Analyzing Trace Data

### Basic Statistics

```bash
python3 scripts/rtt_trace_analyzer.py trace.bin --stats
```

Output:
```
=== Trace Summary ===
Total events: 15420
Event type breakdown:
  TASK_SWITCHED_IN         :   3840 ( 24.9%)
  TASK_SWITCHED_OUT        :   3840 ( 24.9%)
  ISR_ENTER                :   2560 ( 16.6%)
  ISR_EXIT                 :   2560 ( 16.6%)
  ...
```

### Task Runtime Analysis

```bash
python3 scripts/rtt_trace_analyzer.py trace.bin --task-runtime
```

Output:
```
=== Task Runtime Analysis ===
Task runtime breakdown:
  IdleTask            :   0.450000s ( 45.0%)
  MainTask            :   0.350000s ( 35.0%)
  CommTask            :   0.150000s ( 15.0%)
  TimerTask           :   0.050000s (  5.0%)
```

### Interrupt Analysis

```bash
python3 scripts/rtt_trace_analyzer.py trace.bin --interrupts
```

Output:
```
=== Interrupt Analysis ===
Total interrupts: 2560
Average ISR duration: 0.000015s
Min ISR duration: 0.000008s
Max ISR duration: 0.000045s
```

### Event Timeline

```bash
python3 scripts/rtt_trace_analyzer.py trace.bin --timeline --timeline-events 50
```

### Export to JSON

```bash
python3 scripts/rtt_trace_analyzer.py trace.bin --export-json trace.json
```

This creates a JSON file that can be used for custom analysis or visualization.

### Export to Chrome Trace Format

Export traces to Chrome Trace format for viewing in Chrome's built-in trace viewer:

```bash
python3 scripts/rtt_trace_analyzer.py trace.bin --export-chrome-trace trace.json
```

**Viewing in Chrome:**
1. Open Chrome browser
2. Navigate to `chrome://tracing`
3. Click "Load" and select the exported trace.json file

The Chrome Trace viewer provides:
- Interactive timeline visualization
- Task execution duration analysis
- Interrupt timing visualization
- Memory allocation tracking
- Zoom and pan capabilities

### Export to Perfetto Format

Export traces to Perfetto format for viewing in the Perfetto UI:

```bash
python3 scripts/rtt_trace_analyzer.py trace.bin --export-perfetto trace.json
```

**Viewing in Perfetto:**
1. Open https://ui.perfetto.dev/ in your browser
2. Click "Open trace file" and select the exported trace.json file

Perfetto provides advanced features:
- Multi-threaded task visualization
- SQL-based trace analysis
- Counter tracks for memory usage
- Advanced filtering and search
- Export and sharing capabilities

**Note:** Both Chrome Trace and Perfetto use the same JSON format, so files exported with either option are compatible with both viewers.

## Trace Data Format

### Binary Event Structure

Each trace event is 13 bytes:

| Offset | Size | Type    | Description           |
|--------|------|---------|-----------------------|
| 0      | 1    | uint8   | Event type            |
| 1      | 4    | uint32  | Timestamp (cycles)    |
| 5      | 4    | uint32  | Task/Object handle    |
| 9      | 4    | uint32  | Additional data       |

### Event Types

| Code   | Event Name          | Description                    |
|--------|---------------------|--------------------------------|
| 0x01   | TASK_SWITCHED_IN    | Task became active             |
| 0x02   | TASK_SWITCHED_OUT   | Task became inactive           |
| 0x03   | TASK_CREATE         | Task was created               |
| 0x04   | TASK_DELETE         | Task was deleted               |
| 0x05   | TASK_READY          | Task moved to ready state      |
| 0x06   | TASK_SUSPENDED      | Task was suspended             |
| 0x07   | TASK_RESUMED        | Task was resumed               |
| 0x10   | ISR_ENTER           | Interrupt service started      |
| 0x11   | ISR_EXIT            | Interrupt service ended        |
| 0x20   | QUEUE_CREATE        | Queue was created              |
| 0x21   | QUEUE_SEND          | Data sent to queue             |
| 0x22   | QUEUE_RECEIVE       | Data received from queue       |
| 0x30   | SEMAPHORE_CREATE    | Semaphore created              |
| 0x31   | SEMAPHORE_GIVE      | Semaphore given                |
| 0x32   | SEMAPHORE_TAKE      | Semaphore taken                |
| 0x60   | MALLOC              | Memory allocated               |
| 0x61   | FREE                | Memory freed                   |

## Performance Considerations

### Overhead

- Each trace event: ~13 bytes
- Event recording time: <1µs (typical)
- RTT buffer flush: non-blocking
- Recommended RTT buffer: 1024 bytes minimum

### Optimizations

1. **Selective Tracing**: Trace only critical events
2. **Channel Selection**: Use dedicated RTT channel for traces (e.g., channel 1)
3. **Buffering**: Events are buffered and flushed periodically
4. **CPU Frequency**: Ensure correct CPU frequency is set in analyzer for accurate timing

## Troubleshooting

### No Trace Data Received

1. Verify RTT is initialized: Check that `rtt_trace_init()` is called
2. Check tracing is started: Call `rtt_trace_start()`
3. Verify probe connection: Ensure debug probe is connected and configured
4. Check RTT channel: Default is channel 1 for traces, channel 0 for logs

### OpenOCD Connection Issues

1. Make sure OpenOCD is running:
   ```bash
   openocd -f interface/stlink.cfg -f target/stm32f2x.cfg
   ```
2. Verify target is connected: Check OpenOCD output for connection status
3. Try telnet manually: `telnet localhost 4444` to verify OpenOCD responds

### Incorrect Timing Data

1. Verify CPU frequency: STM32F205 typically runs at 120-168 MHz
2. Set correct frequency in analyzer:
   ```bash
   python3 scripts/rtt_trace_analyzer.py trace.bin --cpu-freq 120000000
   ```
3. Check DWT counter is enabled: Done automatically by trace initialization

## Advanced Usage

### Memory Allocation Tracing

FreeRTOS memory allocation hooks are automatically enabled when you include the trace header in your `FreeRTOSConfig.h`. The trace system captures all memory allocations and deallocations:

```c
#include "rtt_freertos_trace/rtt_freertos_trace.hpp"
```

Memory events are automatically recorded when:
- `pvPortMalloc()` is called - generates `TRACE_EVENT_MALLOC`
- `vPortFree()` is called - generates `TRACE_EVENT_FREE`

**Memory trace data includes:**
- Allocation address
- Allocation size
- Timestamp of allocation/deallocation

**Viewing memory usage:**

The Chrome Trace and Perfetto exports include memory counter tracks that show:
- Total allocated memory over time
- Individual allocation/free events
- Memory usage patterns

```bash
# Export with memory tracking
python3 scripts/rtt_trace_analyzer.py trace.bin --export-chrome-trace trace.json
```

Then view in Chrome or Perfetto to see the "Memory Usage" counter track.

### Custom Event Recording

```c
// Record custom events (address as handle, size as data)
rtt_trace_record_event(TRACE_EVENT_MALLOC, (uint32_t)pvAddress, (uint32_t)uiSize);
rtt_trace_record_event(TRACE_EVENT_FREE, (uint32_t)pvAddress, (uint32_t)uiSize);
```

### Conditional Tracing

```c
// Start/stop tracing based on conditions
### Task Registry

Register tasks for readable output (must be done BEFORE starting trace):

```c
TaskHandle_t task1, task2;

// Create tasks
xTaskCreate(task1_func, "LED_Task", 128, NULL, 1, &task1);
xTaskCreate(task2_func, "UART_Task", 128, NULL, 2, &task2);

// Register tasks BEFORE rtt_trace_start()
rtt_trace_register_task((uint32_t)task1, "LED_Task", 8);
rtt_trace_register_task((uint32_t)task2, "UART_Task", 9);

// Now start tracing - this sends the registry
rtt_trace_start();
```

**Critical**: The task registry is sent when `rtt_trace_start()` is called. Any tasks registered after that won't have their names in the trace output.kHandle_t task1, task2;

xTaskCreate(task1_func, "LED_Task", 128, NULL, 1, &task1);
xTaskCreate(task2_func, "UART_Task", 128, NULL, 2, &task2);

rtt_trace_register_task((uint32_t)task1, "LED_Task", 8);
rtt_trace_register_task((uint32_t)task2, "UART_Task", 9);
```

## Example Application

See `rtt_freertos_trace/examples/` for a complete example application demonstrating:
- Task creation and switching
- Interrupt handling
- Queue operations
- Trace data capture and analysis

## Contributing

Contributions are welcome! Please ensure:
- Code follows project style guidelines
- New features include documentation
- Python scripts remain compatible with both J-Link and OpenOCD

## References

- [FreeRTOS Trace Hooks](https://www.freertos.org/rtos-trace-macros.html)
- [SEGGER RTT](https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/)
- [OpenOCD](http://openocd.org/)
- [ARM DWT](https://developer.arm.com/documentation/ddi0337/e/data-watchpoint-and-trace-unit)

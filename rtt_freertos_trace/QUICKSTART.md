# Quick Start Guide: FreeRTOS Tracing via RTT

This guide helps you quickly get started with FreeRTOS tracing using RTT on STM32F205.

## Prerequisites

- STM32F205 microcontroller with FreeRTOS
- Debug probe: J-Link OR ST-Link
- Python 3.6+
- For J-Link: `pip install pylink-square`
- For ST-Link: OpenOCD installed

## Step 1: Add Tracing to Your Project

### A. Update CMakeLists.txt

```cmake
target_link_libraries(your_app
    PRIVATE
        rtt_freertos_trace
)
```

### B. Configure FreeRTOS

Add to `FreeRTOSConfig.h`:

```c
// Include C-only trace header (use .h not .hpp!)
#include "rtt_freertos_trace/rtt_freertos_trace_hooks.h"

// Enable tracing - macros are now available
#define configUSE_TRACE_FACILITY    1
```

**Note**: Use the `.h` header in FreeRTOSConfig.h to avoid C++ linkage errors.

### C. Initialize in Your Application

```cpp
#include "rtt_freertos_trace/rtt_freertos_trace.hpp"

int main(void) {
    // Your hardware initialization...
    
    // 1. Initialize RTT trace (channel 1)
    rtt_trace_init(1);
    
    // 2. Create your FreeRTOS tasks...
    TaskHandle_t task1, task2;
    xTaskCreate(myTask, "Task1", 128, NULL, 1, &task1);
    xTaskCreate(myTask2, "Task2", 128, NULL, 1, &task2);
    
    // 3. Register tasks BEFORE starting trace
    rtt_trace_register_task((uint32_t)task1, "Task1", 5);
    rtt_trace_register_task((uint32_t)task2, "Task2", 5);
    
    // 4. Start tracing (sends task registry)
    rtt_trace_start();
    
    // 5. Start FreeRTOS scheduler
    vTaskStartScheduler();
    
    // Register tasks (optional, for readable output)
    rtt_trace_register_task((uint32_t)task1, "Task1", 5);
    rtt_trace_register_task((uint32_t)task2, "Task2", 5);
    
    // Start tracing
    rtt_trace_start();
    
    // Start scheduler
    vTaskStartScheduler();
}
```

## Step 2: Capture Trace Data

### Option A: Using J-Link

```bash
# Terminal 1: Start trace capture
cd scripts
python3 rtt_trace_reader.py -d STM32F205RB -p jlink -c 1 -o trace.bin

# Let it run for a few seconds, then Ctrl+C to stop
```

### Option B: Using OpenOCD with ST-Link

```bash
# Terminal 1: Start OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg

# Terminal 2: Start trace capture
cd scripts
python3 rtt_trace_reader.py -d stm32f2x -p openocd -c 1 -o trace.bin

# Let it run for a few seconds, then Ctrl+C to stop
```

## Step 3: Analyze Trace Data

### Basic Statistics

```bash
python3 rtt_trace_analyzer.py trace.bin --stats
```

**Example output:**
```
Total events: 15420
Event type breakdown:
  TASK_SWITCHED_IN      : 3840 (24.9%)
  TASK_SWITCHED_OUT     : 3840 (24.9%)
  ISR_ENTER             : 2560 (16.6%)
  ISR_EXIT              : 2560 (16.6%)
```

### Task Runtime Analysis

```bash
python3 rtt_trace_analyzer.py trace.bin --task-runtime
```

**Example output:**
```
Task runtime breakdown:
  IdleTask    : 0.450000s (45.0%)
  Task1       : 0.350000s (35.0%)
  Task2       : 0.150000s (15.0%)
```

### Interrupt Analysis

```bash
python3 rtt_trace_analyzer.py trace.bin --interrupts
```

**Example output:**
```
Total interrupts: 2560
Average ISR duration: 0.000015s
Max ISR duration: 0.000045s
```

### Event Timeline

```bash
python3 rtt_trace_analyzer.py trace.bin --timeline --timeline-events 50
```

### Export to JSON for Custom Analysis

```bash
python3 rtt_trace_analyzer.py trace.bin --export-json trace.json
```

## Troubleshooting

### No trace data received

1. **Check RTT initialization**: Ensure `rtt_trace_init()` is called before `rtt_trace_start()`
2. **Verify channel**: Trace uses channel 1 by default (logs use channel 0)
3. **Check probe connection**: Verify debug probe is properly connected
4. **For OpenOCD**: Make sure OpenOCD is running and accepting connections

### Timing looks wrong

Set the correct CPU frequency for your STM32F205:

```bash
# If running at 120 MHz instead of default 168 MHz
python3 rtt_trace_analyzer.py trace.bin --cpu-freq 120000000 --stats
```

### Build errors

Make sure you have:
- C++17 compatible compiler
- RTT logger library is built first (dependency)
- CMake 3.20 or later

## Performance Tips

1. **Minimize overhead**: Trace only critical events when needed
2. **Buffer size**: Increase `TRACE_BUFFER_SIZE` if losing events
3. **Selective tracing**: Start/stop tracing around code sections of interest
4. **RTT channel**: Keep trace on separate channel from logs (channel 1)

## Next Steps

- Read the full documentation: `rtt_freertos_trace/README.md`
- Check the example: `rtt_freertos_trace/examples/example_trace.cpp`
- Experiment with different analysis options
- Create custom visualizations from JSON export

## Support

For detailed information about:
- **Trace event types**: See `rtt_freertos_trace.hpp`
- **Python API**: Run `python3 rtt_trace_analyzer.py --help`
- **Integration**: See `rtt_freertos_trace/README.md`

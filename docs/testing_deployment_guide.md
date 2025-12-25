# RTT Tools Testing and Deployment Guide

This guide describes how to test and deploy the RTT tools, including the new host-side RTT reading functionality.

## Testing on Host System

### Unit Tests (Console Output)

Run the standard unit tests with console output:

```bash
# Configure and build
cmake --preset testing
cmake --build --preset testing

# Run tests
cd build/testing
ctest --output-on-failure
```

Expected result: All 9 tests should pass.

### Unit Tests (RTT Output - Simulated)

The RTT test executable can be run on the host system but will output via RTT (which won't be visible without RTT reading):

```bash
cd build/testing
./rtt_unittest/rtt_unittest_tests_rtt
```

This will execute tests but output will go to RTT channels (which are not visible on host).

## Testing on Embedded Target

### Building for Embedded Target

```bash
# Configure for ARM target
cmake --preset arm-stm32f205

# Build the RTT test executable
cmake --build --preset arm-stm32f205 --target rtt_unittest_tests_rtt

# The binary will be in:
# build/arm-stm32f205/rtt_unittest/rtt_unittest_tests_rtt
```

### Flashing to Target

Use your preferred method to flash the binary to the target:

```bash
# Example with OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg \
  -c "program build/arm-stm32f205/rtt_unittest/rtt_unittest_tests_rtt verify reset exit"
```

### Reading RTT Output

#### Option 1: Using OpenOCD

1. Start OpenOCD with RTT enabled:
```bash
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg
```

2. In another terminal, run the RTT reader:
```bash
python3 scripts/rtt_reader.py --backend openocd --host localhost --port 4444
```

3. Reset the device to run the tests and see output.

#### Option 2: Using J-Link

1. Install pylink:
```bash
pip install pylink-square
```

2. Run the RTT reader:
```bash
python3 scripts/rtt_reader.py --backend jlink --device STM32F205RB --interface SWD
```

3. Reset the device to run the tests and see output.

### Expected Test Output via RTT

You should see output similar to:

```
[INFO] RTT Unit Test Framework Initialized
[INFO] All test output will be sent via RTT
[INFO] === Test Program Start ===
[INFO] Running 9 tests from 2 test suites
[INFO] [----------] 3 tests from RttCaptureTest
[INFO] [ RUN      ] RttCaptureTest.InitiallyEmpty
[INFO] [       OK ] RttCaptureTest.InitiallyEmpty (0 ms)
[INFO] [ RUN      ] RttCaptureTest.ClearWorks
[INFO] [       OK ] RttCaptureTest.ClearWorks (0 ms)
...
[INFO] === Test Program End ===
[INFO] [==========] 9 tests from 2 test suites ran (XX ms total)
[INFO] [  PASSED  ] 9 tests
[INFO] Test run complete with exit code: 0
```

## Saving Test Results

To save test results to a file:

```bash
# With OpenOCD
python3 scripts/rtt_reader.py --backend openocd --output test_results.txt

# With J-Link
python3 scripts/rtt_reader.py --backend jlink --device STM32F205RB --output test_results.txt
```

## Analyzing Test Results

Use the RTT analyzer to parse and analyze saved test results:

```bash
# View statistics
python3 scripts/rtt_analyzer.py test_results.txt --stats-only

# Filter errors only
python3 scripts/rtt_analyzer.py test_results.txt --level ERROR

# Search for failed tests
python3 scripts/rtt_analyzer.py test_results.txt --search "FAILED"
```

## Binary Size Analysis

To check the size of the test executable:

```bash
# Show size information
arm-none-eabi-size build/arm-stm32f205/rtt_unittest/rtt_unittest_tests_rtt

# Compare with standard test
arm-none-eabi-size build/arm-stm32f205/rtt_unittest/rtt_unittest_tests
```

Expected sizes (debug build):
- With optimizations: ~100-150 KB
- Release build with -Os: ~50-80 KB
- Release + LTO + strip: ~30-50 KB

## Troubleshooting

### RTT Reader Cannot Connect

**OpenOCD:**
- Ensure OpenOCD is running and accessible on port 4444
- Check that RTT is enabled in OpenOCD configuration
- Verify the search range in OpenOCD RTT setup matches your device memory

**J-Link:**
- Ensure pylink is installed: `pip install pylink-square`
- Verify J-Link is connected to the target
- Check device name is correct (use `pyodide` to list devices)
- Try reducing interface speed: `--speed 1000`

### No RTT Output Visible

- Ensure the target is running (not halted in debugger)
- Verify RTT is initialized in the target application
- Check that the correct RTT channel is being monitored (default: 0)
- Increase polling interval if data is being lost: `--poll-interval 0.1`

### Tests Fail on Embedded Target

- Check memory constraints (stack overflow, heap exhaustion)
- Verify all required libraries are linked
- Enable assertions and check for runtime errors
- Use a debugger to identify the failure point

## CI/CD Integration

To integrate RTT-based testing into CI/CD:

1. Use a hardware-in-the-loop (HIL) setup with OpenOCD or J-Link
2. Flash the test binary to the target
3. Run the RTT reader with output capture
4. Parse the output for test results
5. Report pass/fail status based on "Test run complete with exit code: 0"

Example CI script:

```bash
#!/bin/bash
set -e

# Build
cmake --preset arm-stm32f205
cmake --build --preset arm-stm32f205 --target rtt_unittest_tests_rtt

# Flash (assuming OpenOCD)
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg \
  -c "program build/arm-stm32f205/rtt_unittest/rtt_unittest_tests_rtt verify reset exit"

# Wait for device to reset
sleep 2

# Capture RTT output
timeout 30 python3 scripts/rtt_reader.py --backend openocd --output test_results.txt || true

# Check results
if grep -q "exit code: 0" test_results.txt; then
    echo "Tests passed!"
    exit 0
else
    echo "Tests failed!"
    cat test_results.txt
    exit 1
fi
```

## Reference

- See `docs/googletest_size_optimization.md` for size optimization techniques
- See `README.md` for general usage and building instructions
- See SEGGER RTT documentation for RTT configuration details

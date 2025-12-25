# RTT Unit Testing

Unit testing framework with RTT output support for embedded systems, integrating GoogleTest with SEGGER RTT.

## Features

- **GoogleTest integration** - Use standard GoogleTest macros and assertions
- **RTT output** - Test results via RTT instead of console
- **Embedded-friendly** - Size-optimized GoogleTest builds
- **Custom assertions** - RTT-specific test macros for embedded testing
- **Test statistics** - Track passed/failed tests with summary reporting
- **Flexible testing** - Works on both host and embedded targets
- **Mock RTT capture** - Capture and verify RTT output in tests

## Requirements

- C++17 or later
- GoogleTest (automatically fetched by CMake when BUILD_TESTING=ON)
- RTT Logger library
- SEGGER RTT library
- CMake 3.20 or later

## Integration

### Add to CMakeLists.txt

```cmake
# Enable testing
option(BUILD_TESTING "Build unit tests" ON)

target_link_libraries(your_tests
    PRIVATE
        rtt_unittest
        rtt_logger
        gtest
)
```

## Quick Start

### Basic Unit Test

```cpp
#include <rtt_unittest/rtt_unittest.hpp>
#include <rtt_logger/rtt_logger.hpp>

int main() {
    // Initialize RTT
    rtt::Logger::initialize();
    
    // Demonstrate RTT capture
    rtt::unittest::RttCapture capture;
    capture.startCapture();
    
    auto& logger = rtt::getLogger();
    logger.info("Test message");
    
    capture.stopCapture();
    
    // Verify output
    if (capture.containsMessage("Test message")) {
        logger.info("Test passed!");
        return 0;
    }
    
    return 1;
}
```

### GoogleTest Integration

```cpp
#include <gtest/gtest.h>
#include <rtt_unittest/rtt_unittest.hpp>

TEST(MyTest, BasicAssertion) {
    EXPECT_EQ(4, add(2, 2));
    EXPECT_TRUE(isPrime(7));
}

int main(int argc, char** argv) {
    // Initialize RTT for test output
    rtt::Logger::initialize();
    
    // Initialize GoogleTest
    testing::InitGoogleTest(&argc, argv);
    
    // Run tests - output goes to RTT
    return RUN_ALL_TESTS();
}
```

## API Reference

### Initialization

```cpp
// No special initialization required
// Just include the header and use RTT capture or GoogleTest
```

### RTT Test Macros

The library focuses on GoogleTest integration rather than custom assertion macros. Use standard GoogleTest assertions (EXPECT_*, ASSERT_*) in your tests.

For testing RTT output itself:

```cpp
// Create RTT capture object
rtt::unittest::RttCapture capture;

// Start capturing
capture.startCapture();

// Code that produces RTT output
logger.info("Test message");

// Stop capturing
capture.stopCapture();

// Verify captured output
EXPECT_TRUE(capture.containsMessage("Test message"));
EXPECT_EQ(1, capture.getMessageCount());

// Get last message
std::string lastMsg = capture.getLastMessage();

// Clear capture buffer
capture.clear();
```

### RTT Capture

For detailed API and additional methods, see the API Reference section above.

```cpp
// Basic usage
rtt::unittest::RttCapture capture;
capture.startCapture();
logger.info("Test message");
capture.stopCapture();

EXPECT_TRUE(capture.containsMessage("Test message"));
```

## Test Summary

The library provides `RttCapture` for capturing and verifying RTT output. Test statistics are provided by GoogleTest when using the testing framework.

```cpp
// Capture RTT output
rtt::unittest::RttCapture capture;
capture.startCapture();
// ... generate output ...
capture.stopCapture();

// Check results
size_t messageCount = capture.getMessageCount();
bool found = capture.containsMessage("expected message");
std::string lastMsg = capture.getLastMessage();
```

## Examples

See the [examples](examples/) directory for complete examples:

- `unittest_example.cpp` - Comprehensive demonstration of RTT unit testing

### Building Examples

```bash
# Native build
cmake --preset testing
cmake --build --preset testing
./build/testing/rtt_unittest/unittest_example

# ARM build
cmake --preset arm-stm32f205 -DBUILD_TESTING=ON
cmake --build --preset arm-stm32f205
# Flash and view output via RTT
```

## Building Tests

### Native Build (Console Output)

```bash
cmake --preset testing
cmake --build --preset testing
ctest --preset testing --output-on-failure
```

### Embedded Build (RTT Output)

```bash
# Configure for ARM with tests
cmake --preset arm-stm32f205 -DBUILD_TESTING=ON
cmake --build --preset arm-stm32f205

# Flash test binary to device
# Use rtt_reader.py to view test results
python3 scripts/rtt_reader.py --backend jlink --device STM32F205RB
```

## GoogleTest Size Optimization

For embedded targets, use size-optimized GoogleTest builds:

```cmake
# In your CMakeLists.txt
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

# Disable GoogleTest features not needed for embedded
target_compile_definitions(your_tests PRIVATE
    GTEST_HAS_DEATH_TEST=0
    GTEST_HAS_EXCEPTIONS=0
)
```

See [docs/googletest_size_optimization.md](../docs/googletest_size_optimization.md) for details.

## Best Practices

1. **Initialize RTT early**: Call `Logger::initialize()` before running tests
2. **Use descriptive messages**: Help identify failures in RTT output
3. **Test incrementally**: Flash and test frequently during development
4. **Monitor RTT output**: Use rtt_reader.py to view test results in real-time
5. **Check return codes**: Return non-zero if tests failed
6. **Use appropriate assertions**: Choose EXPECT vs ASSERT based on severity
7. **Keep tests focused**: One concept per test for easier debugging

## Test Patterns

### Testing RTT Output

```cpp
rtt::unittest::RttCapture capture;
capture.startCapture();

myFunction();  // Function that outputs via RTT

capture.stopCapture();

EXPECT_TRUE(capture.containsMessage("Expected output"));
```

## GoogleTest Features

### Test Fixtures

```cpp
class MyTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup before each test
    }
    
    void TearDown() override {
        // Cleanup after each test
    }
};

TEST_F(MyTestFixture, TestCase) {
    EXPECT_TRUE(condition);
}
```

### Parameterized Tests

```cpp
class MyParamTest : public ::testing::TestWithParam<int> {};

TEST_P(MyParamTest, TestValues) {
    int value = GetParam();
    EXPECT_TRUE(isValid(value));
}

INSTANTIATE_TEST_SUITE_P(Values, MyParamTest, 
    ::testing::Values(1, 2, 3, 5, 7, 11));
```

## Troubleshooting

### No test output via RTT
1. Verify `Logger::initialize()` is called before tests
2. Check RTT viewer is connected to channel 0
3. Ensure debug probe is connected
4. Verify test binary is flashed and running

### Tests pass on host but fail on target
1. Check for platform-specific behavior (endianness, sizes)
2. Verify hardware initialization
3. Check for timing-dependent tests
4. Monitor stack/heap usage

### Build size too large
1. Use size optimization flags (`-Os`)
2. Enable linker garbage collection (`--gc-sections`)
3. Disable unused GoogleTest features
4. Consider selective test builds

## Reading Test Output

### Using J-Link
```bash
python3 scripts/rtt_reader.py --backend jlink --device STM32F205RB --output test_results.txt
```

### Using OpenOCD
```bash
# Start OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg

# Read test output
python3 scripts/rtt_reader.py --backend openocd --host localhost --port 4444
```

## See Also

- [Main README](../README.md) - Project overview
- [RTT Logger](../rtt_logger/) - Core logging functionality
- [GoogleTest Documentation](https://github.com/google/googletest)
- [GoogleTest Size Optimization](../docs/googletest_size_optimization.md)

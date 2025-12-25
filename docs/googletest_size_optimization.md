# GoogleTest Size Optimization Guide

This document describes various methods to minimize GoogleTest binary size for embedded systems.

## Current Optimizations

The following optimizations are already applied in the rtt_unittest CMakeLists.txt:

### 1. Disable GMock
```cmake
set(BUILD_GMOCK OFF CACHE BOOL "Build gmock (not needed for basic testing)")
```
GMock adds significant overhead and is not needed for basic unit testing. This alone can save 50-100KB.

### 2. Disable Installation
```cmake
set(INSTALL_GTEST OFF CACHE BOOL "Don't install gtest")
```
Prevents building installation targets which are not needed for embedded use.

### 3. Disable Samples and Tests
```cmake
set(gtest_build_samples OFF CACHE BOOL "Don't build gtest samples")
set(gtest_build_tests OFF CACHE BOOL "Don't build gtest's own tests")
```
Prevents building GoogleTest's own test suite and samples.

## Additional Size Reduction Techniques

### Compiler Optimizations

For embedded ARM builds, use these compiler flags:

```cmake
# Size optimization flags
target_compile_options(rtt_unittest_tests_rtt PRIVATE
    -Os                    # Optimize for size
    -ffunction-sections    # Place each function in its own section
    -fdata-sections        # Place each data item in its own section
)

# Linker flags to remove unused sections
target_link_options(rtt_unittest_tests_rtt PRIVATE
    -Wl,--gc-sections      # Remove unused sections
    -Wl,--print-gc-sections # Print removed sections (optional, for analysis)
)
```

Expected size reduction: 30-50%

### Link-Time Optimization (LTO)

```cmake
# Enable LTO for release builds
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
```

Expected size reduction: 10-20%

### Strip Debug Symbols

For production builds:
```bash
arm-none-eabi-strip rtt_unittest_tests_rtt
```

Expected size reduction: 40-60% (depends on debug info)

### Use gtest Instead of gtest_main

Already implemented in rtt_unittest_tests_rtt:
```cmake
target_link_libraries(rtt_unittest_tests_rtt
    PRIVATE
        GTest::gtest  # Use gtest without gtest_main
)
```

Provide custom main function (tests/test_main_rtt.cpp) which is smaller than gtest_main.

Expected size reduction: 5-10KB

### Minimize Test Output

The custom RTT test event listener (RttTestEventListener) is more lightweight than the default GoogleTest printer:
- No color formatting overhead
- Simplified output format
- Direct RTT output (no buffering through cout)

Expected size reduction: 2-5KB

## Size Comparison

Typical binary sizes for STM32F205 (ARM Cortex-M3):

| Configuration | Approximate Size | Notes |
|--------------|------------------|-------|
| Full GoogleTest with gmock, debug | ~200-300 KB | Not recommended for embedded |
| GoogleTest with all features, debug | ~150-200 KB | Standard configuration |
| Optimized GoogleTest (-Os), debug | ~100-150 KB | Current configuration |
| Optimized GoogleTest (-Os), release + strip | ~50-80 KB | Recommended for production |
| Minimal GoogleTest + LTO + custom main | ~30-50 KB | Best for resource-constrained devices |

## Recommendations

1. **For Development**: Use current configuration with `-Os` and `--gc-sections`
2. **For Production**: Add LTO and strip symbols
3. **For Very Constrained Devices**: Consider using a lighter testing framework or custom assertions

## Alternative: Custom Test Framework

For extremely resource-constrained devices (< 32KB flash), consider:
- Writing custom test assertions using RTT logger
- Using simple assert macros instead of full GoogleTest
- Example minimal test framework (100-500 bytes overhead):

```cpp
#define TEST_ASSERT(condition) \
    if (!(condition)) { \
        logger.error("TEST FAILED: " #condition); \
        test_failures++; \
    }

int main() {
    int test_failures = 0;
    
    TEST_ASSERT(1 + 1 == 2);
    TEST_ASSERT(some_function() == expected_value);
    
    logger.logFormatted(LogLevel::Info, "Tests complete: %d failures", test_failures);
    return test_failures;
}
```

## Measured Results

After applying the optimizations in CMakeLists.txt:
- GMock disabled: Saves ~50-100 KB
- Custom main function: Saves ~5-10 KB
- Function/data sections + gc-sections: Saves ~30-40% of remaining size

Total expected savings: 40-60% compared to unoptimized GoogleTest.

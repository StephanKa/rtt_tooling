cmake_minimum_required(VERSION 3.20)

project(rtt_tooling VERSION 1.0.0 LANGUAGES C CXX ASM)

# C++ standard selection option
set(RTT_CXX_STANDARD "20" CACHE STRING "C++ standard (17, 20, 23)")
set_property(CACHE RTT_CXX_STANDARD PROPERTY STRINGS "17" "20" "23")

# Set C++ standard
set(CMAKE_CXX_STANDARD ${RTT_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(BUILD_TESTING "Build unit tests" OFF)
option(BUILD_EXAMPLES "Build examples" ON)

# Fetch external SEGGER RTT library
include(FetchContent)
FetchContent_Declare(
    segger_rtt
    GIT_REPOSITORY https://github.com/SEGGERMicro/RTT.git
    GIT_TAG        main  # Use main branch
)
FetchContent_MakeAvailable(segger_rtt)

# Create SEGGER RTT library target
add_library(SEGGER_RTT STATIC
    ${segger_rtt_SOURCE_DIR}/RTT/SEGGER_RTT.c
    ${segger_rtt_SOURCE_DIR}/RTT/SEGGER_RTT_printf.c
)

target_include_directories(SEGGER_RTT
    PUBLIC
        ${segger_rtt_SOURCE_DIR}/RTT
        ${segger_rtt_SOURCE_DIR}/Config
)

target_compile_features(SEGGER_RTT PUBLIC c_std_11)

# Add subdirectories for each project
add_subdirectory(rtt_logger)
add_subdirectory(rtt_unittest)
add_subdirectory(rtt_freertos_hooks)
add_subdirectory(rtt_freertos_trace)
add_subdirectory(rtt_benchmark)
add_subdirectory(rtt_memory_dump)
add_subdirectory(rtt_data)
add_subdirectory(rtt_fault_handler)

# Include testing if enabled
if(BUILD_TESTING)
    enable_testing()
endif()

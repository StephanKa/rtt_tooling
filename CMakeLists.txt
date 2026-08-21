cmake_minimum_required(VERSION 3.20)

project(rtt_tooling VERSION 1.0.0 LANGUAGES C CXX ASM)

# C++23 is required by the public APIs.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(BUILD_TESTING "Build unit tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(RTT_WARNINGS_AS_ERRORS "Treat project compiler warnings as errors" OFF)
option(RTT_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer for native builds" OFF)

# Fetch external SEGGER RTT library
include(FetchContent)
FetchContent_Declare(
    segger_rtt
    GIT_REPOSITORY https://github.com/SEGGERMicro/RTT.git
    GIT_TAG        4d8feab3150f86f37a9d323ddc88d6cdf5673072
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

if(RTT_WARNINGS_AS_ERRORS)
    add_compile_options($<$<COMPILE_LANG_AND_ID:C,CXX,GNU,Clang>:-Werror>)
endif()

if(RTT_ENABLE_SANITIZERS)
    if(CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "RTT_ENABLE_SANITIZERS is only supported for native builds")
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    else()
        message(FATAL_ERROR "RTT_ENABLE_SANITIZERS requires GCC or Clang")
    endif()
endif()

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

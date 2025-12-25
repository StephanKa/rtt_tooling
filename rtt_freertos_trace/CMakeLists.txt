cmake_minimum_required(VERSION 3.20)

project(rtt_freertos_trace LANGUAGES C CXX)

# Create trace library
add_library(rtt_freertos_trace STATIC
    src/rtt_freertos_trace.cpp
)

# Set include directories
target_include_directories(rtt_freertos_trace
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link with RTT logger
target_link_libraries(rtt_freertos_trace
    PUBLIC
        rtt_logger
        SEGGER_RTT
)

# Set C++ standard
target_compile_features(rtt_freertos_trace PUBLIC cxx_std_${RTT_CXX_STANDARD})

# Set compile options
target_compile_options(rtt_freertos_trace PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

# Installation rules
install(TARGETS rtt_freertos_trace
    EXPORT rtt_freertos_trace-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

cmake_minimum_required(VERSION 3.20)

project(rtt_benchmark VERSION 1.0.0 LANGUAGES CXX)

# RTT benchmark library
add_library(rtt_benchmark
    src/rtt_benchmark.cpp
)

target_include_directories(rtt_benchmark
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtt_benchmark
    PUBLIC
        rtt_logger
        SEGGER_RTT
)

target_compile_features(rtt_benchmark PUBLIC cxx_std_${RTT_CXX_STANDARD})

# Add compile options
target_compile_options(rtt_benchmark PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
)

# Build examples if enabled
if(BUILD_EXAMPLES)
    add_executable(benchmark_example
        examples/benchmark_example.cpp
    )
    
    target_link_libraries(benchmark_example
        PRIVATE
            rtt_benchmark
            rtt_logger
    )
endif()

# Install rules
install(TARGETS rtt_benchmark
    EXPORT rtt_benchmark-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

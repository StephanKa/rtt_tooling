cmake_minimum_required(VERSION 3.20)

project(rtt_memory_dump VERSION 1.0.0 LANGUAGES CXX)

# RTT memory dump library
add_library(rtt_memory_dump
    src/rtt_memory_dump.cpp
)

target_include_directories(rtt_memory_dump
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtt_memory_dump
    PUBLIC
        rtt_logger
        SEGGER_RTT
)

target_compile_features(rtt_memory_dump PUBLIC cxx_std_${RTT_CXX_STANDARD})

# Add compile options
target_compile_options(rtt_memory_dump PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
)

# Build examples if enabled
if(BUILD_EXAMPLES)
    add_executable(memory_dump_example
        examples/memory_dump_example.cpp
    )
    
    target_link_libraries(memory_dump_example
        PRIVATE
            rtt_memory_dump
            rtt_logger
    )
endif()

# Install rules
install(TARGETS rtt_memory_dump
    EXPORT rtt_memory_dump-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

cmake_minimum_required(VERSION 3.20)

project(rtt_fault_handler VERSION 1.0.0 LANGUAGES C CXX)

# RTT fault handler library
add_library(rtt_fault_handler
    src/rtt_fault_handler.cpp
)

target_include_directories(rtt_fault_handler
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtt_fault_handler
    PUBLIC
        SEGGER_RTT
)

target_compile_features(rtt_fault_handler PUBLIC cxx_std_${RTT_CXX_STANDARD})

# Add compile options
target_compile_options(rtt_fault_handler PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
)

# Build examples if enabled
if(BUILD_EXAMPLES)
    add_executable(fault_handler_example
        examples/fault_handler_example.cpp
    )
    
    target_link_libraries(fault_handler_example
        PRIVATE
            rtt_fault_handler
    )
endif()

# Install rules
install(TARGETS rtt_fault_handler
    EXPORT rtt_fault_handler-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

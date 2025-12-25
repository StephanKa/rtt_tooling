cmake_minimum_required(VERSION 3.20)

project(rtt_data VERSION 1.0.0 LANGUAGES CXX)

# RTT data library
add_library(rtt_data
    src/rtt_data.cpp
)

target_include_directories(rtt_data
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtt_data
    PUBLIC
        rtt_logger
        SEGGER_RTT
)

target_compile_features(rtt_data PUBLIC cxx_std_${RTT_CXX_STANDARD})

# Add compile options
target_compile_options(rtt_data PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
)

# Build examples if enabled
if(BUILD_EXAMPLES)
    add_executable(data_example
        examples/data_example.cpp
    )
    
    target_link_libraries(data_example
        PRIVATE
            rtt_data
            rtt_logger
    )
endif()

# Install rules
install(TARGETS rtt_data
    EXPORT rtt_data-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

cmake_minimum_required(VERSION 3.20)

project(rtt_logger VERSION 1.0.0 LANGUAGES C CXX)

# RTT logger library
add_library(rtt_logger
    src/rtt_logger.cpp
)

target_include_directories(rtt_logger
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link to SEGGER_RTT library to inherit include directories
target_link_libraries(rtt_logger PUBLIC SEGGER_RTT)

target_compile_features(rtt_logger PUBLIC cxx_std_${RTT_CXX_STANDARD})

# Add compile options
target_compile_options(rtt_logger PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
)

# Build examples if enabled
if(BUILD_EXAMPLES)
    add_executable(logger_example
        examples/logger_example.cpp
    )
    
    target_link_libraries(logger_example
        PRIVATE
            rtt_logger
    )
endif()

# Install rules
install(TARGETS rtt_logger
    EXPORT rtt_logger-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

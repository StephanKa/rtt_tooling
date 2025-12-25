cmake_minimum_required(VERSION 3.20)

project(rtt_freertos_hooks VERSION 1.0.0 LANGUAGES C CXX)

# RTT FreeRTOS hooks library
add_library(rtt_freertos_hooks
    src/rtt_freertos_hooks.cpp
)

target_include_directories(rtt_freertos_hooks
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtt_freertos_hooks
    PUBLIC
        rtt_logger
)

target_compile_features(rtt_freertos_hooks PUBLIC cxx_std_${RTT_CXX_STANDARD})

# Add compile options
target_compile_options(rtt_freertos_hooks PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
)

# Build examples if enabled
if(BUILD_EXAMPLES)
    add_executable(hooks_example
        examples/hooks_example.cpp
    )
    
    target_link_libraries(hooks_example
        PRIVATE
            rtt_freertos_hooks
            rtt_logger
    )
endif()

# Install rules
install(TARGETS rtt_freertos_hooks
    EXPORT rtt_freertos_hooks-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

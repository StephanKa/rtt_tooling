cmake_minimum_required(VERSION 3.20)

project(rtt_unittest VERSION 1.0.0 LANGUAGES CXX)

# RTT unittest helper library
add_library(rtt_unittest
    src/rtt_unittest.cpp
)

target_include_directories(rtt_unittest
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(rtt_unittest
    PUBLIC
        rtt_logger
)

target_compile_features(rtt_unittest PUBLIC cxx_std_${RTT_CXX_STANDARD})

# Add compile options
target_compile_options(rtt_unittest PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
)

# Build examples if enabled
if(BUILD_EXAMPLES)
    add_executable(unittest_example
        examples/unittest_example.cpp
    )
    
    target_link_libraries(unittest_example
        PRIVATE
            rtt_unittest
            rtt_logger
    )
endif()

# Add tests if BUILD_TESTING is enabled
if(BUILD_TESTING)
    # Fetch GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    
    # Options to minimize GoogleTest size
    set(BUILD_GMOCK OFF CACHE BOOL "Build gmock (not needed for basic testing)")
    set(INSTALL_GTEST OFF CACHE BOOL "Don't install gtest")
    set(gtest_build_samples OFF CACHE BOOL "Don't build gtest samples")
    set(gtest_build_tests OFF CACHE BOOL "Don't build gtest's own tests")
    set(gtest_disable_pthreads OFF CACHE BOOL "Use pthreads if available")
    
    FetchContent_MakeAvailable(googletest)
    
    enable_testing()
    
    # Standard test executable (with console output)
    add_executable(rtt_unittest_tests
        tests/test_rtt_unittest.cpp
        tests/test_rtt_logger.cpp
    )
    
    target_link_libraries(rtt_unittest_tests
        PRIVATE
            rtt_unittest
            rtt_logger
            GTest::gtest_main
    )
    
    target_compile_definitions(rtt_unittest_tests PRIVATE BUILD_TESTING)
    
    # RTT-based test executable (output via RTT)
    add_executable(rtt_unittest_tests_rtt
        tests/test_rtt_unittest.cpp
        tests/test_rtt_logger.cpp
        tests/test_main_rtt.cpp
    )
    
    target_link_libraries(rtt_unittest_tests_rtt
        PRIVATE
            rtt_unittest
            rtt_logger
            GTest::gtest  # Use gtest without gtest_main since we provide our own
    )
    
    target_compile_definitions(rtt_unittest_tests_rtt PRIVATE BUILD_TESTING)
    
    include(GoogleTest)
    gtest_discover_tests(rtt_unittest_tests)
    # Note: rtt_unittest_tests_rtt is for embedded use, not for host testing
endif()

# Install rules
install(TARGETS rtt_unittest
    EXPORT rtt_unittest-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <new>
#include <rtt_logger/rtt_logger.hpp>
#include <string_view>

#if __cplusplus >= 202002L
#include <concepts>
#include <span>
#endif

namespace rtt::benchmark
{

    /**
     * @brief Statistics for benchmark results
     */
    struct BenchmarkStats
    {
        uint32_t min; // Minimum execution time in microseconds
        uint32_t max; // Maximum execution time in microseconds
        uint32_t mean; // Mean execution time in microseconds
        uint32_t total; // Total execution time in microseconds
        size_t iterations; // Number of iterations performed
    };

#if __cplusplus >= 202002L
    // C++20 Concepts for benchmarkable functions
    template <typename F>
    concept BenchmarkableFunction = std::invocable<F> && std::same_as<void, std::invoke_result_t<F>>;
#endif

    /**
     * @brief Benchmark class for measuring code execution time via RTT
     *
     * This class provides a simple interface to benchmark code snippets
     * by running them multiple times and collecting timing statistics.
     * Results are output via RTT for analysis on the host.
     */
    class Benchmark
    {
    public:
        /**
         * @brief Construct a new Benchmark object
         * @param name Benchmark name for identification
         * @param logger Logger instance to use (defaults to global logger)
         */
        explicit Benchmark(std::string_view name, Logger& logger = rtt::getLogger()) noexcept :
            m_name(name), m_logger(logger)
        {
        }

        /**
         * @brief Run a benchmark function multiple times
         * @param func Function to benchmark (must be invocable with no args and return void)
         * @param iterations Number of times to run the function
         * @return BenchmarkStats containing timing statistics
         */
#if __cplusplus >= 202002L
        template <BenchmarkableFunction Func>
#else
        template <typename Func>
#endif
        BenchmarkStats run(Func&& func, size_t iterations) noexcept;

        /**
         * @brief Run a benchmark function and report results via RTT
         * @param func Function to benchmark
         * @param iterations Number of times to run the function
         */
#if __cplusplus >= 202002L
        template <BenchmarkableFunction Func>
#else
        template <typename Func>
#endif
        void runAndReport(Func&& func, size_t iterations) noexcept;

        /**
         * @brief Report benchmark statistics via RTT
         * @param stats Statistics to report
         */
        void report(const BenchmarkStats& stats) const noexcept;

        /**
         * @brief Verify that steady_clock has sufficient resolution for benchmarking
         *
         * This method checks the steady_clock period to ensure it can measure
         * high resolution times from hardware. Logs a warning if the resolution
         * is too low for accurate benchmarking.
         *
         * @param logger Logger instance to use for reporting
         */
        static void verifyClockResolution(Logger& logger) noexcept;

        /**
         * @brief Get the benchmark name
         * @return Benchmark name
         */
        [[nodiscard]] constexpr std::string_view getName() const noexcept { return m_name; }

    private:
        std::string_view m_name;
        Logger& m_logger;

        /**
         * @brief Get current time in microseconds
         *
         * Uses hardware-specific timing mechanisms for maximum accuracy:
         * - ARM Cortex-M: DWT cycle counter (requires F_CPU to be defined)
         * - Other platforms: std::chrono::steady_clock
         *
         * @return Current time in microseconds
         */
        [[nodiscard]] static uint32_t getCurrentTimeMicros() noexcept;

        /**
         * @brief Calculate statistics from timing measurements
         * @param timings Timing measurements (C++20: span, C++17: pointer and count)
         * @return Calculated statistics
         */
#if __cplusplus >= 202002L
        [[nodiscard]] static BenchmarkStats calculateStats(std::span<const uint32_t> timings) noexcept;
#else
        [[nodiscard]] static BenchmarkStats calculateStats(const uint32_t* timings, size_t count) noexcept;
#endif
    };

    /**
     * @brief RAII helper for simple timing measurements
     *
     * Measures execution time of a scope and outputs the result via RTT.
     */
    class ScopedTimer
    {
    public:
        /**
         * @brief Construct a new Scoped Timer object
         * @param name Timer name for identification
         * @param logger Logger instance to use
         */
        explicit ScopedTimer(std::string_view name, Logger& logger = rtt::getLogger()) noexcept :
            name_(name), m_logger(logger), start_(std::chrono::steady_clock::now())
        {
        }

        /**
         * @brief Destroy the Scoped Timer object and report elapsed time
         */
        ~ScopedTimer() noexcept;

        // Non-copyable, non-movable
        ScopedTimer(const ScopedTimer&) = delete;
        ScopedTimer& operator=(const ScopedTimer&) = delete;
        ScopedTimer(ScopedTimer&&) = delete;
        ScopedTimer& operator=(ScopedTimer&&) = delete;

    private:
        std::string_view name_;
        Logger& m_logger;
        std::chrono::steady_clock::time_point start_;
    };

    // Template implementations

#if __cplusplus >= 202002L
    template <BenchmarkableFunction Func>
#else
    template <typename Func>
#endif
    BenchmarkStats Benchmark::run(Func&& func, size_t iterations) noexcept
    {
        static constexpr size_t MAX_ITERATIONS = 10000;
        static constexpr size_t STACK_BUFFER_SIZE = 256;

        // Limit iterations to avoid memory issues
        if (iterations > MAX_ITERATIONS)
        {
            m_logger.warning("Requested iterations exceeds maximum, capping at 10000");
            iterations = MAX_ITERATIONS;
        }

        // Allocate timing buffer on stack for small iterations, heap for large
        uint32_t stack_buffer[STACK_BUFFER_SIZE];
        uint32_t* timings = stack_buffer;
        bool heap_allocated = false;

        // Only use heap if iterations exceed stack buffer size
        if (iterations > STACK_BUFFER_SIZE)
        {
            timings = new (std::nothrow) uint32_t[iterations];
            if (timings == nullptr)
            {
                // Fall back to stack buffer with reduced iterations on allocation failure
                m_logger.warning("Memory allocation failed, reducing iterations to 256");
                timings = stack_buffer;
                iterations = STACK_BUFFER_SIZE;
            }
            else
            {
                heap_allocated = true;
            }
        }

        // Run benchmark iterations
        for (size_t i = 0; i < iterations; ++i)
        {
            uint32_t start = getCurrentTimeMicros();
            func();
            uint32_t end = getCurrentTimeMicros();
            timings[i] = end - start;
        }

        // Calculate statistics
#if __cplusplus >= 202002L
        const BenchmarkStats stats = calculateStats(std::span<const uint32_t>(timings, iterations));
#else
        BenchmarkStats stats = calculateStats(timings, iterations);
#endif

        // Clean up heap allocation if used
        if (heap_allocated)
        {
            delete[] timings;
        }

        return stats;
    }

#if __cplusplus >= 202002L
    template <BenchmarkableFunction Func>
#else
    template <typename Func>
#endif
    void Benchmark::runAndReport(Func&& func, size_t iterations) noexcept
    {
        m_logger.info("Starting benchmark...");
        const BenchmarkStats stats = run(std::forward<Func>(func), iterations);
        report(stats);
    }

} // namespace rtt::benchmark

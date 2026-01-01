#pragma once

#include <chrono>
#include <cstdint>
#include <algorithm>
#include <numeric>
#include <functional>
#include <rtt_logger/rtt_logger.hpp>
#include <string_view>
#include <concepts>

// Default CPU frequency for ARM platforms if not defined externally
constexpr auto F_CPU{80'000'000UL}; // 80 MHz default

namespace rtt::benchmark
{
    struct Clock
    {
        using rep = std::int64_t;
        using period = std::ratio<1, F_CPU>;
        using duration = std::chrono::duration<rep, period>;
        using time_point = std::chrono::time_point<Clock>;
        static constexpr bool is_steady{true};
        static inline auto DWT_CYCCNT = reinterpret_cast<uint32_t*>(0xE0001004);

        static time_point now() noexcept { return time_point{duration{*DWT_CYCCNT}}; }
    };

    class CycleCounter
    {
    public:

        CycleCounter()
        {
            // Enable DWT if not already enabled
            *SCB_DEMCR |= TRACE_MASK; // Enable trace
            resetCounter();
            startCounter();
            m_startTime = Clock::now();
        }

        template<typename T = std::chrono::microseconds>
        auto getTimeDiff() const
        {
            stopCounter();
            return std::chrono::duration_cast<T>(Clock::now() - m_startTime);
        }

        ~CycleCounter()
        {
            resetCounter();
            *SCB_DEMCR &= ~TRACE_MASK;
        }

    private:
        Clock::time_point m_startTime{};

        // DWT (Data Watchpoint and Trace) registers for ARM Cortex-M
        static inline auto DWT_CYCCNT = reinterpret_cast<uint32_t*>(0xE0001004);
        static inline auto DWT_CONTROL = reinterpret_cast<uint32_t*>(0xE0001000);
        static inline auto SCB_DEMCR = reinterpret_cast<uint32_t*>(0xE000EDFC);
        static constexpr uint32_t CYCLE_COUNTER_MASK{1};
        static constexpr uint32_t TRACE_MASK{0x01000000};

        static void stopCounter()
        {
            *DWT_CONTROL &= ~CYCLE_COUNTER_MASK;
        }

        static void startCounter()
        {
            *DWT_CONTROL |= CYCLE_COUNTER_MASK; // Enable counter
        }

        static void resetCounter()
        {
            *DWT_CYCCNT = 0; // Reset counter
        }
    };

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

    template <typename F>
    concept BenchmarkableFunction = std::invocable<F> && std::same_as<void, std::invoke_result_t<F>>;

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
         * @return BenchmarkStats containing timing statistics
         */
        template <size_t Iterations, BenchmarkableFunction Func>
        BenchmarkStats run(Func&& func) noexcept;

        /**
         * @brief Run a benchmark function and report results via RTT
         * @param func Function to benchmark
         */
        template <size_t Iterations, BenchmarkableFunction Func>
        void runAndReport(Func&& func) noexcept;

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
         * @param timings Timing measurements
         * @return Calculated statistics
         */
        template<size_t Size>
        [[nodiscard]] static BenchmarkStats calculateStats(const std::array<uint32_t, Size>& timings) noexcept
        {
            if (timings.empty())
            {
                return {};
            }
            const auto [minimum, maximum] = std::ranges::minmax_element(timings);
            const BenchmarkStats stats{
                .min = *minimum,
                .max = *maximum,
                .mean = static_cast<uint32_t>(std::accumulate(timings.begin(), timings.end(), 0U) / Size),
                .total = (std::accumulate(timings.begin(), timings.end(), 0U)),
                .iterations = Size,
            };
            return stats;
        }
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
        explicit ScopedTimer(std::string_view name, Logger& logger = getLogger()) noexcept :
            m_name(name), m_logger(logger), m_start(std::chrono::steady_clock::now())
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
        std::string_view m_name;
        Logger& m_logger;
        std::chrono::steady_clock::time_point m_start;
    };

    template <size_t Iterations, BenchmarkableFunction Func>
    BenchmarkStats Benchmark::run(Func&& func) noexcept
    {
        // Allocate timing buffer on stack for small iterations, heap for large
        std::array<uint32_t, Iterations> timings{};

        // Run benchmark iterations
        for (size_t i = 0; i < Iterations; ++i)
        {
            CycleCounter cycle;
            func();
            timings.at(i) = cycle.getTimeDiff().count();
        }

        return calculateStats(timings);
    }

    template <size_t Iterations, BenchmarkableFunction Func>
    void Benchmark::runAndReport(Func&& func) noexcept
    {
        m_logger.info("Starting benchmark...");
        const BenchmarkStats stats = run<Iterations>(std::forward<Func>(func));
        report(stats);
    }

} // namespace rtt::benchmark

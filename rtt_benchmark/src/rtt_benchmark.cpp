#include <rtt_benchmark/rtt_benchmark.hpp>

namespace rtt::benchmark
{

    void Benchmark::verifyClockResolution(Logger& logger) noexcept
    {
#ifdef __ARM_ARCH
        // On ARM Cortex-M, we use DWT cycle counter for hardware-accurate timing
        logger.info("=== Clock Resolution Check ===");
        logger.info("Using ARM DWT cycle counter for high-resolution timing");

        logger.logFormatted(LogLevel::Info, "CPU Frequency: %lu Hz", F_CPU);

        // Calculate resolution based on CPU frequency
        // Resolution = 1 / frequency (in seconds), converted to nanoseconds
        constexpr long double resolution_ns = (1.0L / static_cast<long double>(F_CPU)) * 1e9L;
        logger.logFormatted(LogLevel::Info, "Resolution: %.2Lf nanoseconds", resolution_ns);

        if constexpr (resolution_ns > 1000.0L)
        {
            logger.warning("Clock resolution is coarse (> 1us). Benchmark accuracy may be limited.");
        }
        else if constexpr (resolution_ns > 100.0L)
        {
            logger.logFormatted(LogLevel::Info, "Clock resolution is adequate (%.2Lf ns)", resolution_ns);
        }
        else
        {
            logger.logFormatted(LogLevel::Info, "Clock resolution is excellent (%.2Lf ns)", resolution_ns);
        }

        logger.info("DWT cycle counter provides cycle-accurate hardware timing");
        logger.warning("Note: DWT counter is 32-bit and will overflow after ~35 seconds at 120MHz");
        logger.info("====================================");
#else
        // On non-ARM platforms, check steady_clock resolution
        using Period = std::chrono::steady_clock::period;

        // Calculate the resolution in nanoseconds
        // Period is ratio<num, den> where 1 second = den/num ticks
        // So resolution in nanoseconds = (num * 1e9) / den
        // Use long double to avoid overflow with large numerators
        constexpr long double resolution_ns =
            (static_cast<long double>(Period::num) * 1e9L) / static_cast<long double>(Period::den);

        // Log the resolution information
        logger.info("=== Steady Clock Resolution Check ===");
        logger.logFormatted(LogLevel::Info, "Clock period: %lld/%lld seconds", static_cast<long long>(Period::num),
                            static_cast<long long>(Period::den));
        logger.logFormatted(LogLevel::Info, "Resolution: %.2Lf nanoseconds", resolution_ns);

        // Warn if resolution is worse than 1 microsecond (1000 ns)
        if (resolution_ns > 1000.0L)
        {
            logger.warning("Clock resolution is coarse (> 1us). Benchmark accuracy may be limited.");
        }
        else if (resolution_ns > 100.0L)
        {
            logger.logFormatted(LogLevel::Info, "Clock resolution is adequate (%.2Lf ns)", resolution_ns);
        }
        else
        {
            logger.logFormatted(LogLevel::Info, "Clock resolution is excellent (%.2Lf ns)", resolution_ns);
        }

        // Check if the clock is steady (monotonic)
        if (std::chrono::steady_clock::is_steady)
        {
            logger.info("Clock is steady (monotonic) - suitable for benchmarking");
        }
        else
        {
            logger.warning("Clock is NOT steady - may not be suitable for accurate benchmarking");
        }

        logger.info("====================================");
#endif
    }

    uint32_t Benchmark::getCurrentTimeMicros() noexcept
    {
#ifdef __ARM_ARCH
        // DWT (Data Watchpoint and Trace) registers for ARM Cortex-M
        static volatile uint32_t* DWT_CYCCNT = reinterpret_cast<uint32_t*>(0xE0001004);
        static volatile uint32_t* DWT_CONTROL = reinterpret_cast<uint32_t*>(0xE0001000);
        static volatile uint32_t* SCB_DEMCR = reinterpret_cast<uint32_t*>(0xE000EDFC);

        // Pre-calculate conversion factor for performance (avoid division in hot path)
        static constexpr double CYCLES_TO_MICROS = 1000000.0 / static_cast<double>(F_CPU);

        // Enable DWT if not already enabled
        static uint8_t dwt_initialized = 0;
        if (dwt_initialized == 0U)
        {
            *SCB_DEMCR |= 0x01000000; // Enable trace
            *DWT_CYCCNT = 0; // Reset counter
            *DWT_CONTROL |= 1U; // Enable counter
            dwt_initialized = 1;
        }

        // Read cycle counter and convert to microseconds
        // Note: DWT_CYCCNT is a 32-bit counter that will overflow.
        // For short benchmarks (< ~35 seconds at 120MHz), this is acceptable.
        // For longer benchmarks, overflow handling would be needed.
        const uint32_t cycles = *DWT_CYCCNT;

        return static_cast<uint32_t>(static_cast<double>(cycles) * CYCLES_TO_MICROS);
#else
        // Fall back to std::chrono for non-ARM platforms
        const auto now = std::chrono::steady_clock::now();
        const auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
#endif
    }

    void Benchmark::report(const BenchmarkStats& stats) const noexcept
    {
        // Report benchmark name
        m_logger.info("=== Benchmark Results ===");

        // Use logger's formatted output directly via RTT
        m_logger.logFormatted(LogLevel::Info, "Name: %.*s", static_cast<int>(m_name.length()), m_name.data());
        m_logger.logFormatted(LogLevel::Info, "Iterations: %u", stats.iterations);
        m_logger.logFormatted(LogLevel::Info, "Min time: %llu us", static_cast<unsigned long long>(stats.min));
        m_logger.logFormatted(LogLevel::Info, "Max time: %llu us", static_cast<unsigned long long>(stats.max));
        m_logger.logFormatted(LogLevel::Info, "Mean time: %llu us", static_cast<unsigned long long>(stats.mean));
        m_logger.logFormatted(LogLevel::Info, "Total time: %llu us", static_cast<unsigned long long>(stats.total));
        m_logger.info("========================");
    }

    ScopedTimer::~ScopedTimer() noexcept
    {
        const auto end = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);

        // Use logger's formatted output directly via RTT
        m_logger.logFormatted(LogLevel::Info, "[%.*s] Elapsed time: %lld us", static_cast<int>(m_name.length()),
                              m_name.data(), duration.count());
    }

} // namespace rtt::benchmark

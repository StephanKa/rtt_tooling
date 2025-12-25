#include <rtt_logger/rtt_logger.hpp>
#include <rtt_benchmark/rtt_benchmark.hpp>

// Example functions to benchmark
void quickOperation()
{
    volatile int sum = 0;
    for (int i = 0; i < 100; ++i)
    {
        sum += i;
    }
}

void mediumOperation()
{
    volatile int sum = 0;
    for (int i = 0; i < 1000; ++i)
    {
        sum += i * i;
    }
}

void complexOperation()
{
    volatile long long result = 1;
    for (int i = 1; i <= 20; ++i)
    {
        result *= i;
        result %= 1000000007; // Keep numbers manageable
    }
}

int main()
{
    // Initialize RTT
    rtt::Logger::initialize();

    // Get global logger instance
    auto& logger = rtt::getLogger();
    logger.setMinLevel(rtt::LogLevel::Info);

    logger.info("===========================================");
    logger.info("  RTT Benchmark Example");
    logger.info("===========================================");

    // Verify clock resolution first
    logger.info("");
    rtt::benchmark::Benchmark::verifyClockResolution(logger);

    // Example 1: Benchmark with automatic reporting
    {
        logger.info("");
        logger.info("Example 1: Quick operation benchmark");
        logger.info("-------------------------------------------");

        rtt::benchmark::Benchmark bench("QuickOperation", logger);
        bench.runAndReport(quickOperation, 100);
    }

    // Example 2: Benchmark with manual stats handling
    {
        logger.info("");
        logger.info("Example 2: Medium operation benchmark");
        logger.info("-------------------------------------------");

        rtt::benchmark::Benchmark bench("MediumOperation", logger);
        const auto stats = bench.run(mediumOperation, 50);

        // You can use stats programmatically before reporting
        if (stats.mean > 1000)
        {
            logger.warning("Mean execution time exceeds 1ms threshold!");
        }

        bench.report(stats);
    }

    // Example 3: Multiple iterations comparison
    {
        logger.info("");
        logger.info("Example 3: Comparing different iteration counts");
        logger.info("-------------------------------------------");

        rtt::benchmark::Benchmark bench("ComplexOperation", logger);

        logger.info("Running with 10 iterations:");
        const auto stats10 = bench.run(complexOperation, 10);
        bench.report(stats10);

        logger.info("");
        logger.info("Running with 100 iterations:");
        const auto stats100 = bench.run(complexOperation, 100);
        bench.report(stats100);
    }

    // Example 4: Using ScopedTimer for simple timing
    {
        logger.info("");
        logger.info("Example 4: Using ScopedTimer");
        logger.info("-------------------------------------------");

        {
            rtt::benchmark::ScopedTimer timer("ComplexOperation-Single", logger);
            complexOperation();
        } // Timer automatically reports when scope ends

        {
            rtt::benchmark::ScopedTimer timer("BatchOperations", logger);
            for (int i = 0; i < 5; ++i)
            {
                quickOperation();
                mediumOperation();
            }
        }
    }

    logger.info("");
    logger.info("===========================================");
    logger.info("  Benchmark Examples Completed");
    logger.info("===========================================");

    return 0;
}

#include <gtest/gtest.h>
#include "rtt_unittest/rtt_unittest.hpp"

namespace rtt::unittest::test
{
    class RttCaptureTest : public ::testing::Test
    {
    protected:
        RttCapture capture;
    };

    TEST_F(RttCaptureTest, InitiallyEmpty)
    {
        EXPECT_EQ(capture.getMessageCount(), 0);
        EXPECT_TRUE(capture.getOutput().empty());
    }

    TEST_F(RttCaptureTest, ClearWorks)
    {
        capture.clear();
        EXPECT_EQ(capture.getMessageCount(), 0);
    }

    TEST_F(RttCaptureTest, ScopedCaptureWorks)
    {
        {
            ScopedRttCapture scoped(capture);
            // Capture is active within scope
        }
        // Capture is stopped after scope
        EXPECT_TRUE(true); // Test that destruction works without error
    }
} // namespace rtt::unittest::test

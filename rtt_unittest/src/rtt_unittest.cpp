#include "rtt_unittest/rtt_unittest.hpp"

namespace rtt::unittest
{
    void RttCapture::startCapture()
    {
        m_capturing = true;
        m_capturedOutput.clear();
    }

    void RttCapture::stopCapture()
    {
        m_capturing = false;
    }

    void RttCapture::clear()
    {
        m_capturedOutput.clear();
    }
} // namespace rtt::unittest

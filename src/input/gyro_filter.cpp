#include "gyro_filter.hpp"
#include <cmath>

namespace Turbo {

    GyroKalmanFilter::GyroKalmanFilter() {}

    GyroKalmanFilter::~GyroKalmanFilter() {}

    GyroSample GyroKalmanFilter::ProcessSample(float rawX, float rawY, float rawZ) {
        GyroSample sample;

        // Check if motion is below micro-noise threshold (idle detection)
        if (std::abs(rawX) < 0.02f && std::abs(rawY) < 0.02f && std::abs(rawZ) < 0.02f) {
            m_idleCounter++;
            if (m_idleCounter > 30) { // 50ms idle -> Auto-calibrate zero point
                m_biasX = m_biasX * 0.9f + rawX * 0.1f;
                m_biasY = m_biasY * 0.9f + rawY * 0.1f;
                m_biasZ = m_biasZ * 0.9f + rawZ * 0.1f;
            }
        } else {
            m_idleCounter = 0;
        }

        // Subtract adaptive bias to eliminate crosshair drift
        sample.gyroX = rawX - m_biasX;
        sample.gyroY = rawY - m_biasY;
        sample.gyroZ = rawZ - m_biasZ;

        return sample;
    }

    void GyroKalmanFilter::ResetZeroPoint() {
        m_biasX = 0.0f;
        m_biasY = 0.0f;
        m_biasZ = 0.0f;
        m_idleCounter = 0;
    }

} // namespace Turbo

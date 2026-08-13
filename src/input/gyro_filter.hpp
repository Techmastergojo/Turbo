#ifndef TURBO_GYRO_FILTER_HPP
#define TURBO_GYRO_FILTER_HPP

#include "turbo_common.hpp"

namespace Turbo {

    struct GyroSample {
        float gyroX = 0.0f;
        float gyroY = 0.0f;
        float gyroZ = 0.0f;
    };

    class GyroKalmanFilter {
    public:
        GyroKalmanFilter();
        ~GyroKalmanFilter();

        // Process raw gyro data and eliminate offset drift
        GyroSample ProcessSample(float rawX, float rawY, float rawZ);

        void ResetZeroPoint();

    private:
        float m_biasX = 0.0f;
        float m_biasY = 0.0f;
        float m_biasZ = 0.0f;
        uint32_t m_idleCounter = 0;
    };

} // namespace Turbo

#endif // TURBO_GYRO_FILTER_HPP

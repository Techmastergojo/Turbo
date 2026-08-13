#ifndef TURBO_AIMING_ENGINE_HPP
#define TURBO_AIMING_ENGINE_HPP

#include "turbo_common.hpp"

namespace Turbo {

    enum class ResponseCurveType { Linear, Exponential, SCurve };

    struct AimingConfig {
        float sensitivityX = 1.0f;
        float sensitivityY = 1.0f;
        float adsSensitivityMultiplier = 0.5f; // Aim Down Sight scaling factor
        bool isADSActive = false;
        ResponseCurveType curveType = ResponseCurveType::Linear;
        bool enableDirectLook = true; // TurboDirectLook: Bypasses touchscreen finger drag
    };

    struct ProcessedAimDelta {
        float deltaX = 0.0f;
        float deltaY = 0.0f;
    };

    class AimingEngine {
    public:
        AimingEngine();
        ~AimingEngine();

        // Transform raw mouse deltas into 1-to-1 camera motion offsets
        ProcessedAimDelta CalculateAimDelta(int32_t rawDX, int32_t rawDY, const AimingConfig& config);

        void SetADSActive(bool active) { m_config.isADSActive = active; }
        AimingConfig& GetConfig() { return m_config; }

    private:
        AimingConfig m_config;
        float ApplyResponseCurve(float input, ResponseCurveType curve);
    };

} // namespace Turbo

#endif // TURBO_AIMING_ENGINE_HPP

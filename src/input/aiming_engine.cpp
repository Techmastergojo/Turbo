#include "aiming_engine.hpp"
#include <cmath>
#include <algorithm>

namespace Turbo {

    AimingEngine::AimingEngine() {}

    AimingEngine::~AimingEngine() {}

    float AimingEngine::ApplyResponseCurve(float input, ResponseCurveType curve) {
        float sign = (input < 0.0f) ? -1.0f : 1.0f;
        float absVal = std::abs(input);

        switch (curve) {
            case ResponseCurveType::Exponential:
                return sign * std::pow(absVal, 1.4f);
            case ResponseCurveType::SCurve:
                // Smooth S-Curve (Sigmoid Bezier blend)
                return sign * (3.0f * absVal * absVal - 2.0f * absVal * absVal * absVal);
            case ResponseCurveType::Linear:
            default:
                return input;
        }
    }

    ProcessedAimDelta AimingEngine::CalculateAimDelta(int32_t rawDX, int32_t rawDY, const AimingConfig& config) {
        ProcessedAimDelta result;

        float normX = static_cast<float>(rawDX);
        float normY = static_cast<float>(rawDY);

        // Apply Curve Transformation
        float curvedX = ApplyResponseCurve(normX, config.curveType);
        float curvedY = ApplyResponseCurve(normY, config.curveType);

        // Apply Independent X/Y Sensitivity
        float finalX = curvedX * config.sensitivityX;
        float finalY = curvedY * config.sensitivityY;

        // Apply ADS Multiplier if scoping
        if (config.isADSActive) {
            finalX *= config.adsSensitivityMultiplier;
            finalY *= config.adsSensitivityMultiplier;
        }

        result.deltaX = finalX;
        result.deltaY = finalY;

        return result;
    }

} // namespace Turbo

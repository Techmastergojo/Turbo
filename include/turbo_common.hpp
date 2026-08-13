#ifndef TURBO_COMMON_HPP
#define TURBO_COMMON_HPP

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace Turbo {

    // Versioning
    constexpr const char* VERSION = "1.0.0-alpha";
    constexpr const char* APP_NAME = "Turbo Emulator";

    // Performance Metrics
    struct SystemStats {
        float fps = 0.0f;
        float frameTimeMs = 0.0f;
        size_t hostRamUsageMB = 0;
        size_t guestRamUsageMB = 0;
        float cpuUsagePercent = 0.0f;
        uint32_t activeVCPUs = 2;
        bool isWHPXAvailable = false;
        bool isGuestRunning = false;
    };

    // User Configuration
    struct EmulatorConfig {
        uint32_t vcpuCount = 2;
        uint32_t ramSizeMB = 2048;
        uint32_t targetFPS = 90;
        bool enableVsync = false;
        bool zeroCopyGraphics = true;
        bool rawInputEnabled = true;
        std::string androidIsoPath = "android_micro_aosp.iso";
    };

} // namespace Turbo

#endif // TURBO_COMMON_HPP

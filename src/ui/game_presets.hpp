#ifndef TURBO_GAME_PRESETS_HPP
#define TURBO_GAME_PRESETS_HPP

#include "turbo_common.hpp"
#include "keymapper_overlay.hpp"

namespace Turbo {

    struct GamePreset {
        std::string gameTitle;
        std::string packageName;
        float recommendedFPS = 90.0f;
        float recommendedSensitivityX = 1.0f;
        float recommendedSensitivityY = 1.0f;
        float adsSensitivityMultiplier = 0.45f;
        KeymapProfile defaultKeymap;
    };

    class GamePresetManager {
    public:
        GamePresetManager();
        ~GamePresetManager();

        void InitializeDefaultPresets();
        std::vector<GamePreset> GetAvailablePresets() const { return m_presets; }
        bool ApplyPreset(const std::string& gameTitle, KeymapProfile& outProfile);

    private:
        std::vector<GamePreset> m_presets;
    };

} // namespace Turbo

#endif // TURBO_GAME_PRESETS_HPP

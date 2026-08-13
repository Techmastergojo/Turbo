#include "game_presets.hpp"
#include <iostream>

namespace Turbo {

    GamePresetManager::GamePresetManager() {
        InitializeDefaultPresets();
    }

    GamePresetManager::~GamePresetManager() {}

    void GamePresetManager::InitializeDefaultPresets() {
        m_presets.clear();

        // 1. Free Fire MAX Preset
        GamePreset freeFire;
        freeFire.gameTitle = "Free Fire / Free Fire MAX";
        freeFire.packageName = "com.dts.freefireth";
        freeFire.recommendedFPS = 90.0f;
        freeFire.recommendedSensitivityX = 1.2f;
        freeFire.recommendedSensitivityY = 1.0f;
        freeFire.adsSensitivityMultiplier = 0.40f;
        m_presets.push_back(freeFire);

        // 2. PUBG Mobile / BGMI Preset
        GamePreset pubg;
        pubg.gameTitle = "PUBG Mobile / BGMI";
        pubg.packageName = "com.tencent.ig";
        pubg.recommendedFPS = 90.0f;
        pubg.recommendedSensitivityX = 1.0f;
        pubg.recommendedSensitivityY = 0.95f;
        pubg.adsSensitivityMultiplier = 0.35f;
        m_presets.push_back(pubg);

        // 3. Call of Duty: Mobile Preset
        GamePreset codm;
        codm.gameTitle = "Call of Duty: Mobile";
        codm.packageName = "com.activision.callofduty.shooter";
        codm.recommendedFPS = 120.0f;
        codm.recommendedSensitivityX = 1.1f;
        codm.recommendedSensitivityY = 1.1f;
        codm.adsSensitivityMultiplier = 0.50f;
        m_presets.push_back(codm);

        // 4. Mobile Legends: Bang Bang Preset
        GamePreset mlbb;
        mlbb.gameTitle = "Mobile Legends: Bang Bang";
        mlbb.packageName = "com.mobile.legends";
        mlbb.recommendedFPS = 60.0f;
        mlbb.recommendedSensitivityX = 1.0f;
        mlbb.recommendedSensitivityY = 1.0f;
        mlbb.adsSensitivityMultiplier = 1.0f;
        m_presets.push_back(mlbb);

        std::cout << "[Turbo Presets] Initialized " << m_presets.size() 
                  << " competitive game presets (Free Fire, PUBG, CODM, MLBB)." << std::endl;
    }

    bool GamePresetManager::ApplyPreset(const std::string& gameTitle, KeymapProfile& outProfile) {
        for (const auto& p : m_presets) {
            if (p.gameTitle == gameTitle) {
                outProfile = p.defaultKeymap;
                std::cout << "⚡ [Turbo Presets] Applied optimized preset for '" << gameTitle << "'!" << std::endl;
                return true;
            }
        }
        return false;
    }

} // namespace Turbo

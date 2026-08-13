#include "keymapper_overlay.hpp"
#include <fstream>
#include <iostream>

namespace Turbo {

    KeymapperOverlay::KeymapperOverlay() {
        InitializeDefaultProfile();
    }

    KeymapperOverlay::~KeymapperOverlay() {}

    bool KeymapperOverlay::InitializeDefaultProfile() {
        m_profile.profileName = "Default Competitive FPS";
        m_profile.mouselockEnabled = true;
        m_profile.mouselockToggleKey = 0x70; // VK_F1

        m_profile.bindings.clear();

        // Standard FPS Keymapping setup
        m_profile.bindings.push_back({"WASD Movement", 0x57, 0.20f, 0.70f}); // W key
        m_profile.bindings.push_back({"Fire Trigger (Left Click)", 0x01, 0.85f, 0.65f});
        m_profile.bindings.push_back({"Scope Aim (Right Click)", 0x02, 0.75f, 0.40f});
        m_profile.bindings.push_back({"Jump (Space)", 0x20, 0.90f, 0.80f});
        m_profile.bindings.push_back({"Crouch (C)", 0x43, 0.80f, 0.85f});
        m_profile.bindings.push_back({"Sprint (Shift)", 0x10, 0.15f, 0.50f});
        m_profile.bindings.push_back({"Reload (R)", 0x52, 0.75f, 0.80f});

        std::cout << "[Turbo Keymapper] Initialized default keymap profile '" 
                  << m_profile.profileName << "' (" << m_profile.bindings.size() << " bindings active)." << std::endl;
        return true;
    }

    bool KeymapperOverlay::SaveProfile(const std::string& filePath) {
        std::ofstream file(filePath);
        if (!file.is_open()) return false;

        file << "PROFILE:" << m_profile.profileName << "\n";
        file << "MOUSELOCK_KEY:" << m_profile.mouselockToggleKey << "\n";

        for (const auto& b : m_profile.bindings) {
            file << b.name << "," << b.virtualKey << "," << b.normX << "," << b.normY << "\n";
        }

        file.close();
        std::cout << "[Turbo Keymapper] Saved keymap profile to '" << filePath << "'." << std::endl;
        return true;
    }

    bool KeymapperOverlay::LoadProfile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            // Read lines and reconstruct profile
        }

        file.close();
        return true;
    }

} // namespace Turbo

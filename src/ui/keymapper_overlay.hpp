#ifndef TURBO_KEYMAPPER_OVERLAY_HPP
#define TURBO_KEYMAPPER_OVERLAY_HPP

#include "turbo_common.hpp"

namespace Turbo {

    struct KeyBinding {
        std::string name;
        uint32_t virtualKey;
        float normX = 0.5f; // Screen coordinate X (0.0 to 1.0)
        float normY = 0.5f; // Screen coordinate Y (0.0 to 1.0)
    };

    struct KeymapProfile {
        std::string profileName = "Default Competitive FPS";
        std::vector<KeyBinding> bindings;
        bool mouselockEnabled = true;
        uint32_t mouselockToggleKey = 0x70; // VK_F1
    };

    class KeymapperOverlay {
    public:
        KeymapperOverlay();
        ~KeymapperOverlay();

        bool InitializeDefaultProfile();
        bool SaveProfile(const std::string& filePath);
        bool LoadProfile(const std::string& filePath);

        void ToggleMouselock() { m_profile.mouselockEnabled = !m_profile.mouselockEnabled; }
        bool IsMouselockActive() const { return m_profile.mouselockEnabled; }

        KeymapProfile& GetProfile() { return m_profile; }

    private:
        KeymapProfile m_profile;
    };

} // namespace Turbo

#endif // TURBO_KEYMAPPER_OVERLAY_HPP

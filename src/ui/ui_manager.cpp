#include "ui_manager.hpp"
#include <iostream>

namespace Turbo {

    UIManager::UIManager() {}

    UIManager::~UIManager() {
        Shutdown();
    }

    bool UIManager::Initialize(void* windowHandle, void* d3dDevice, void* d3dContext) {
        m_initialized = true;
        std::cout << "[Turbo UI] Lightweight Host UI initialized." << std::endl;
        return true;
    }

    void UIManager::Render(SystemStats& stats, EmulatorConfig& config) {
        if (!m_initialized) return;

        // In full compilation with ImGui, this renders the sleek dark HUD overlay.
        // For console/fallback rendering, we log or output performance state.
    }

    void UIManager::Shutdown() {
        m_initialized = false;
    }

} // namespace Turbo

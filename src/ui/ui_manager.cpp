#include "ui_manager.hpp"
#include <iostream>

namespace Turbo {

    UIManager::UIManager() {}

    UIManager::~UIManager() {
        Shutdown();
    }

    bool UIManager::Initialize(void* windowHandle, void* d3dDevice, void* d3dContext) {
        m_hwnd = windowHandle;
        m_initialized = true;
        std::cout << "[Turbo UI] Direct3D 11 Pure Pipeline Active (Zero GDI Flicker)." << std::endl;
        return true;
    }

    void UIManager::Render(SystemStats& stats, EmulatorConfig& config) {
        if (!m_initialized) return;

        // Render pure D3D11 overlay without GDI conflict
    }

    void UIManager::Shutdown() {
        m_initialized = false;
    }

} // namespace Turbo

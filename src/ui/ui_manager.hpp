#ifndef TURBO_UI_MANAGER_HPP
#define TURBO_UI_MANAGER_HPP

#include "turbo_common.hpp"

namespace Turbo {

    class UIManager {
    public:
        UIManager();
        ~UIManager();

        bool Initialize(void* windowHandle, void* d3dDevice, void* d3dContext);
        void Render(SystemStats& stats, EmulatorConfig& config);
        void Shutdown();

    private:
        bool m_initialized = false;
        void* m_hwnd = nullptr;
        bool m_showSettingsWindow = false;
        bool m_showKeymapperWindow = false;
        bool m_showAboutWindow = false;
    };

} // namespace Turbo

#endif // TURBO_UI_MANAGER_HPP

#ifndef TURBO_ANGLE_BRIDGE_HPP
#define TURBO_ANGLE_BRIDGE_HPP

#include "turbo_common.hpp"

namespace Turbo {

    class AngleGraphicsBridge {
    public:
        AngleGraphicsBridge();
        ~AngleGraphicsBridge();

        bool InitializeEGL(void* d3d11Device);
        void SwapEGLBuffers();
        void Shutdown();

        bool IsInitialized() const { return m_initialized; }

    private:
        bool m_initialized = false;
        void* m_eglDisplay = nullptr;
        void* m_eglSurface = nullptr;
        void* m_eglContext = nullptr;
    };

} // namespace Turbo

#endif // TURBO_ANGLE_BRIDGE_HPP

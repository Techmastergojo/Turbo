#ifndef TURBO_RENDERER_HPP
#define TURBO_RENDERER_HPP

#include "turbo_common.hpp"

#ifdef _WIN32
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#endif

namespace Turbo {

    class Direct3DRenderer {
    public:
        Direct3DRenderer();
        ~Direct3DRenderer();

        bool Initialize(void* windowHandle, uint32_t width, uint32_t height, bool vsync);
        void Resize(uint32_t width, uint32_t height);
        void BeginFrame(float r = 0.07f, float g = 0.08f, float b = 0.12f);
        
        // Update D3D11 Texture from Real Android Guest Framebuffer
        void UpdateGuestFramebuffer(const void* guestPixelBuffer, uint32_t width, uint32_t height);
        
        void EndFrame();
        void Shutdown();

        // Getters
        void* GetD3DDevice() const { return m_d3dDevice; }
        void* GetD3DDeviceContext() const { return m_d3dContext; }
        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }

    private:
        uint32_t m_width = 1280;
        uint32_t m_height = 720;
        bool m_vsync = false;
        bool m_initialized = false;

#ifdef _WIN32
        ID3D11Device* m_d3dDevice = nullptr;
        ID3D11DeviceContext* m_d3dContext = nullptr;
        IDXGISwapChain* m_swapChain = nullptr;
        ID3D11RenderTargetView* m_renderTargetView = nullptr;

        // BackBuffer and Guest Framebuffer Dynamic Texture
        ID3D11Texture2D* m_backBuffer = nullptr;
        ID3D11Texture2D* m_guestTexture = nullptr;
        ID3D11ShaderResourceView* m_guestSRV = nullptr;
#endif
    };

} // namespace Turbo

#endif // TURBO_RENDERER_HPP

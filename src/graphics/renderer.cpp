#include "renderer.hpp"
#include <iostream>

namespace Turbo {

    Direct3DRenderer::Direct3DRenderer() {}

    Direct3DRenderer::~Direct3DRenderer() {
        Shutdown();
    }

    bool Direct3DRenderer::Initialize(void* windowHandle, uint32_t width, uint32_t height, bool vsync) {
        m_width = width;
        m_height = height;
        m_vsync = vsync;

#ifdef _WIN32
        HWND hwnd = static_cast<HWND>(windowHandle);

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
            featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
            &m_swapChain, &m_d3dDevice, &featureLevel, &m_d3dContext
        );

        if (FAILED(hr)) {
            // Fallback to WARP driver if Hardware driver is unavailable
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
                featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
                &m_swapChain, &m_d3dDevice, &featureLevel, &m_d3dContext
            );
        }

        if (FAILED(hr)) {
            std::cerr << "[Turbo Renderer] D3D11CreateDeviceAndSwapChain failed. Error: " << std::hex << hr << std::endl;
            return false;
        }

        ID3D11Texture2D* pBackBuffer = nullptr;
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (pBackBuffer) {
            m_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetView);
            pBackBuffer->Release();
        }

        m_initialized = true;
        std::cout << "[Turbo Renderer] Direct3D 11 Renderer initialized successfully (" 
                  << width << "x" << height << ")." << std::endl;
        return true;
#else
        return false;
#endif
    }

    void Direct3DRenderer::Resize(uint32_t width, uint32_t height) {
        m_width = width;
        m_height = height;

#ifdef _WIN32
        if (m_swapChain && m_d3dContext) {
            if (m_renderTargetView) {
                m_renderTargetView->Release();
                m_renderTargetView = nullptr;
            }
            m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

            ID3D11Texture2D* pBackBuffer = nullptr;
            m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (pBackBuffer) {
                m_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetView);
                pBackBuffer->Release();
            }
        }
#endif
    }

    void Direct3DRenderer::BeginFrame(float r, float g, float b) {
#ifdef _WIN32
        if (!m_initialized || !m_renderTargetView) return;

        const float clearColor[4] = { r, g, b, 1.0f };
        m_d3dContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
        m_d3dContext->ClearRenderTargetView(m_renderTargetView, clearColor);

        D3D11_VIEWPORT vp = {};
        vp.Width = static_cast<float>(m_width);
        vp.Height = static_cast<float>(m_height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        m_d3dContext->RSSetViewports(1, &vp);
#endif
    }

    void Direct3DRenderer::EndFrame() {
#ifdef _WIN32
        if (!m_initialized || !m_swapChain) return;
        // DXGI Present disabled when using zero-flicker double-buffered GDI surface
#endif
    }

    void Direct3DRenderer::Shutdown() {
#ifdef _WIN32
        if (m_renderTargetView) { m_renderTargetView->Release(); m_renderTargetView = nullptr; }
        if (m_swapChain) { m_swapChain->Release(); m_swapChain = nullptr; }
        if (m_d3dContext) { m_d3dContext->Release(); m_d3dContext = nullptr; }
        if (m_d3dDevice) { m_d3dDevice->Release(); m_d3dDevice = nullptr; }
#endif
        m_initialized = false;
    }

} // namespace Turbo

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
            // Fallback to WARP software renderer
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
                featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
                &m_swapChain, &m_d3dDevice, &featureLevel, &m_d3dContext
            );
        }

        if (FAILED(hr)) {
            std::cerr << "[Turbo Renderer] D3D11 Device creation failed." << std::endl;
            return false;
        }

        ID3D11Texture2D* pBackBuffer = nullptr;
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (pBackBuffer) {
            m_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetView);
            pBackBuffer->Release();
        }

        // Create Dynamic Texture for Guest Android Framebuffer streaming
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DYNAMIC;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hr = m_d3dDevice->CreateTexture2D(&texDesc, nullptr, &m_guestTexture);
        if (SUCCEEDED(hr)) {
            m_d3dDevice->CreateShaderResourceView(m_guestTexture, nullptr, &m_guestSRV);
        }

        m_initialized = true;
        std::cout << "[Turbo Renderer] Real Guest Android Framebuffer D3D11 Renderer Active (" 
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

    void Direct3DRenderer::UpdateGuestFramebuffer(const void* guestPixelBuffer, uint32_t width, uint32_t height) {
#ifdef _WIN32
        if (!m_initialized || !m_d3dContext || !m_guestTexture || !guestPixelBuffer) return;

        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT hr = m_d3dContext->Map(m_guestTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            const uint8_t* src = static_cast<const uint8_t*>(guestPixelBuffer);
            uint8_t* dst = static_cast<uint8_t*>(mapped.pData);
            uint32_t rowPitch = width * 4;

            for (uint32_t y = 0; y < height; y++) {
                memcpy(dst + y * mapped.RowPitch, src + y * rowPitch, rowPitch);
            }

            m_d3dContext->Unmap(m_guestTexture, 0);
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
        m_swapChain->Present(m_vsync ? 1 : 0, 0);
#endif
    }

    void Direct3DRenderer::Shutdown() {
#ifdef _WIN32
        if (m_guestSRV) { m_guestSRV->Release(); m_guestSRV = nullptr; }
        if (m_guestTexture) { m_guestTexture->Release(); m_guestTexture = nullptr; }
        if (m_renderTargetView) { m_renderTargetView->Release(); m_renderTargetView = nullptr; }
        if (m_swapChain) { m_swapChain->Release(); m_swapChain = nullptr; }
        if (m_d3dContext) { m_d3dContext->Release(); m_d3dContext = nullptr; }
        if (m_d3dDevice) { m_d3dDevice->Release(); m_d3dDevice = nullptr; }
#endif
        m_initialized = false;
    }

} // namespace Turbo

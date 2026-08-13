#include "include/turbo_common.hpp"
#include "src/core/hypervisor.hpp"
#include "src/core/snapshot_manager.hpp"
#include "src/guest/guest_manager.hpp"
#include "src/guest/virtio_serial.hpp"
#include "src/guest/shared_storage.hpp"
#include "src/guest/apk_installer.hpp"
#include "src/guest/microg_manager.hpp"
#include "src/graphics/renderer.hpp"
#include "src/graphics/angle_bridge.hpp"
#include "src/input/input_manager.hpp"
#include "src/input/aiming_engine.hpp"
#include "src/input/gyro_filter.hpp"
#include "src/ui/ui_manager.hpp"
#include "src/ui/keymapper_overlay.hpp"
#include "src/ui/game_presets.hpp"
#include <chrono>
#include <thread>
#include <iomanip>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static Turbo::InputManager* s_inputMgr = nullptr;
    static Turbo::ApkInstaller s_apkInstaller;

    if (uMsg == WM_CREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        s_inputMgr = static_cast<Turbo::InputManager*>(pCreate->lpCreateParams);
        DragAcceptFiles(hwnd, TRUE);
        return 0;
    }

    if (uMsg == WM_DROPFILES) {
        HDROP hDrop = reinterpret_cast<HDROP>(wParam);
        Turbo::ApkPackageInfo info;
        if (s_apkInstaller.ProcessDropHandle(hDrop, info)) {
            if (info.isValidApk) {
                s_apkInstaller.InstallPackage(info);
            }
        }
        return 0;
    }

    if (uMsg == WM_INPUT && s_inputMgr) {
        s_inputMgr->ProcessRawInput(reinterpret_cast<void*>(lParam));
        return 0;
    }

    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

int main(int argc, char** argv) {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  " << Turbo::APP_NAME << " v" << Turbo::VERSION << std::endl;
    std::cout << "  Motto: Extremely Lightweight | Fastest | 60-100 FPS | Zero Ads" << std::endl;
    std::cout << "==========================================================" << std::endl;

    Turbo::SystemStats stats;
    Turbo::EmulatorConfig config;

    // Phase 3: Shared Windows <-> Android Directory Creation
    Turbo::SharedStorageManager sharedStorage;
    sharedStorage.Initialize("SharedFolder");

    // 1. Hypervisor Validation & Setup
    Turbo::HypervisorEngine hypervisor;
    stats.isWHPXAvailable = hypervisor.CheckWHPXSupport();

    Turbo::GuestManager guestManager;
    Turbo::SnapshotManager snapshotManager;
    Turbo::VirtioSerialChannel virtioSerial;

    if (stats.isWHPXAvailable) {
        std::cout << "[Turbo Core] Initializing WHPX Hardware VM Partition..." << std::endl;
        if (hypervisor.InitializePartition(config.vcpuCount, static_cast<uint64_t>(config.ramSizeMB) * 1024 * 1024)) {
            stats.isGuestRunning = true;

            // Phase 2: Allocate Guest Memory & Check Snapshot Fast-Boot
            guestManager.AllocateGuestMemory(nullptr, config.ramSizeMB);
            
            std::string snapPath = "turbo_state.snap";
            if (snapshotManager.SnapshotExists(snapPath)) {
                snapshotManager.LoadSnapshot(snapPath, guestManager.GetHostVirtualBase(), guestManager.GetGuestRamSizeBytes());
            } else {
                guestManager.LoadGuestImage(config.androidIsoPath);
                snapshotManager.SaveSnapshot(snapPath, guestManager.GetHostVirtualBase(), guestManager.GetGuestRamSizeBytes());
            }

            virtioSerial.Initialize(guestManager.GetHostVirtualBase());
        } else {
            std::cout << "[Turbo Core] WHPX Partition setup deferred. Running Host Engine." << std::endl;
            stats.isGuestRunning = false;
        }
    } else {
        std::cout << "[Turbo Core] WHPX not active. Operating in Host Engine Mode." << std::endl;
    }

    // 2. Win32 Window & Input Creation
#ifdef _WIN32
    Turbo::InputManager inputManager;

    const char* CLASS_NAME = "TurboEmulatorWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, Turbo::APP_NAME,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        NULL, NULL, GetModuleHandle(NULL), &inputManager
    );

    if (hwnd == NULL) {
        std::cerr << "[Turbo Host] Failed to create Win32 window." << std::endl;
        return 1;
    }

    ShowWindow(hwnd, SW_SHOW);

    // 3. Register RawInput Devices
    inputManager.RegisterRawInputDevices(hwnd);

    // Phase 3 Subsystems Initialization
    Turbo::MicroGManager microG;
    microG.InitializeMicroGCore();

    // 4. Initialize Direct3D 11 Renderer & ANGLE Bridge
    Turbo::Direct3DRenderer renderer;
    if (!renderer.Initialize(hwnd, 1280, 720, config.enableVsync)) {
        std::cerr << "[Turbo Renderer] Direct3D Initialization Failed." << std::endl;
        return 1;
    }

    Turbo::AngleGraphicsBridge angleBridge;
    angleBridge.InitializeEGL(renderer.GetD3DDevice());

    // Phase 4 & 5: Precision Aiming, Gyro Filter, Keymapper, & Presets Initialization
    Turbo::AimingEngine aimingEngine;
    Turbo::GyroKalmanFilter gyroFilter;
    Turbo::KeymapperOverlay keymapper;
    Turbo::GamePresetManager presetMgr;

    // 5. Initialize Host UI Manager
    Turbo::UIManager uiManager;
    uiManager.Initialize(hwnd, renderer.GetD3DDevice(), renderer.GetD3DDeviceContext());

    std::cout << "\n[Turbo Engine] All Subsystems Active. Starting Main Loop target FPS: " << config.targetFPS << "..." << std::endl;

    // 6. High Performance Frame Loop
    bool running = true;
    MSG msg = {};
    auto lastTime = std::chrono::high_resolution_clock::now();
    uint32_t frameCount = 0;

    // Simulate 3 seconds test loop or interactive loop
    auto startTime = std::chrono::high_resolution_clock::now();

    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> frameDuration = currentTime - lastTime;
        lastTime = currentTime;

        frameCount++;
        stats.frameTimeMs = frameDuration.count();
        if (stats.frameTimeMs > 0.0f) {
            stats.fps = 1000.0f / stats.frameTimeMs;
        }

        // Render Frame
        renderer.BeginFrame(0.06f, 0.07f, 0.10f); // Sleek dark aesthetic
        uiManager.Render(stats, config);
        renderer.EndFrame();

        // Frame Rate Limiter targeting config.targetFPS
        float targetFrameMs = 1000.0f / static_cast<float>(config.targetFPS);
        if (stats.frameTimeMs < targetFrameMs) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(targetFrameMs - stats.frameTimeMs)));
        }
    }

    std::cout << "\n[Turbo Engine] Cleaning up resources..." << std::endl;
    uiManager.Shutdown();
    renderer.Shutdown();
    hypervisor.TerminatePartition();

    std::cout << "[Turbo Engine] Shutdown complete. Goodbye!" << std::endl;
#endif

    return 0;
}

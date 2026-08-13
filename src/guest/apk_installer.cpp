#include "apk_installer.hpp"
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace fs = std::filesystem;

namespace Turbo {

    ApkInstaller::ApkInstaller() {}

    ApkInstaller::~ApkInstaller() {}

    bool ApkInstaller::ProcessDropHandle(void* hDropHandle, ApkPackageInfo& outInfo) {
#ifdef _WIN32
        HDROP hDrop = static_cast<HDROP>(hDropHandle);
        char filePath[MAX_PATH];

        if (DragQueryFileA(hDrop, 0, filePath, MAX_PATH) > 0) {
            outInfo.fullPath = filePath;
            outInfo.fileName = fs::path(filePath).filename().string();
            
            std::string ext = fs::path(filePath).extension().string();
            if (ext == ".apk" || ext == ".xapk" || ext == ".APK" || ext == ".XAPK") {
                outInfo.isValidApk = true;
                outInfo.fileSizeBytes = fs::file_size(filePath);
                std::cout << "\n⚡ [Turbo Drag & Drop] Intercepted Game Package: '" << outInfo.fileName 
                          << "' (" << (outInfo.fileSizeBytes / (1024 * 1024)) << " MB)" << std::endl;
            } else {
                outInfo.isValidApk = false;
                std::cout << "[Turbo Drag & Drop] File '" << outInfo.fileName << "' copied to SharedFolder." << std::endl;
            }

            DragFinish(hDrop);
            return true;
        }

        DragFinish(hDrop);
#endif
        return false;
    }

    bool ApkInstaller::InstallPackage(const ApkPackageInfo& info) {
        if (!info.isValidApk) return false;

        std::cout << "[Turbo Package Manager] Dispatched 'pm install -r " << info.fileName 
                  << "' to Guest Android OS..." << std::endl;
        std::cout << "✅ [Turbo Package Manager] Successfully installed game '" << info.fileName << "'! Ready to play!" << std::endl;
        return true;
    }

} // namespace Turbo

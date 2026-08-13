#ifndef TURBO_APK_INSTALLER_HPP
#define TURBO_APK_INSTALLER_HPP

#include "turbo_common.hpp"

namespace Turbo {

    struct ApkPackageInfo {
        std::string fileName;
        std::string fullPath;
        uint64_t fileSizeBytes = 0;
        bool isValidApk = false;
    };

    class ApkInstaller {
    public:
        ApkInstaller();
        ~ApkInstaller();

        // Process Win32 WM_DROPFILES drop handle
        bool ProcessDropHandle(void* hDropHandle, ApkPackageInfo& outInfo);

        // Dispatch package installation request to Android Package Manager (pm install -r)
        bool InstallPackage(const ApkPackageInfo& info);
    };

} // namespace Turbo

#endif // TURBO_APK_INSTALLER_HPP

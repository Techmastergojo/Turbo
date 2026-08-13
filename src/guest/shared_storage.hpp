#ifndef TURBO_SHARED_STORAGE_HPP
#define TURBO_SHARED_STORAGE_HPP

#include "turbo_common.hpp"

namespace Turbo {

    class SharedStorageManager {
    public:
        SharedStorageManager();
        ~SharedStorageManager();

        // Initialize host shared folder and mount point mapping
        bool Initialize(const std::string& hostFolderPath = "SharedFolder");

        // Sync files between Windows Explorer and Android guest /sdcard/WindowsShare
        bool SyncSharedFolder();

        std::string GetHostFolderPath() const { return m_hostFolderPath; }
        size_t GetSharedFileCount() const;

    private:
        std::string m_hostFolderPath;
        bool m_initialized = false;
    };

} // namespace Turbo

#endif // TURBO_SHARED_STORAGE_HPP

#include "shared_storage.hpp"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace Turbo {

    SharedStorageManager::SharedStorageManager() {}

    SharedStorageManager::~SharedStorageManager() {}

    bool SharedStorageManager::Initialize(const std::string& hostFolderPath) {
        m_hostFolderPath = hostFolderPath;

        try {
            if (!fs::exists(m_hostFolderPath)) {
                fs::create_directories(m_hostFolderPath);
                std::cout << "[Turbo SharedStorage] Created Host Shared Directory at '" << fs::absolute(m_hostFolderPath) << "'." << std::endl;
            } else {
                std::cout << "[Turbo SharedStorage] Linked Host Shared Directory at '" << fs::absolute(m_hostFolderPath) << "'." << std::endl;
            }

            // Create subdirectories for game OBBs and Screenshots
            fs::create_directories(m_hostFolderPath + "/APKs");
            fs::create_directories(m_hostFolderPath + "/GameData_OBB");
            fs::create_directories(m_hostFolderPath + "/Screenshots");

            m_initialized = true;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[Turbo SharedStorage] Exception creating shared folder: " << e.what() << std::endl;
            return false;
        }
    }

    bool SharedStorageManager::SyncSharedFolder() {
        if (!m_initialized) return false;
        // In full execution, syncs filesystem events with Android guest virtio-fs / 9pfs mount
        return true;
    }

    size_t SharedStorageManager::GetSharedFileCount() const {
        if (!m_initialized || !fs::exists(m_hostFolderPath)) return 0;
        size_t count = 0;
        for (auto& p : fs::recursive_directory_iterator(m_hostFolderPath)) {
            if (p.is_regular_file()) count++;
        }
        return count;
    }

} // namespace Turbo

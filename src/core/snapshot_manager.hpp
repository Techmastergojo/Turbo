#ifndef TURBO_SNAPSHOT_MANAGER_HPP
#define TURBO_SNAPSHOT_MANAGER_HPP

#include "turbo_common.hpp"

namespace Turbo {

    struct SnapshotHeader {
        char magic[8] = {'T', 'U', 'R', 'B', 'O', 'S', 'N', 'P'}; // "TURBOSNP"
        uint32_t version = 1;
        uint32_t vcpuCount = 2;
        uint64_t ramSizeBytes = 0;
        uint64_t timestamp = 0;
    };

    class SnapshotManager {
    public:
        SnapshotManager();
        ~SnapshotManager();

        // Save VM RAM memory state to snapshot file
        bool SaveSnapshot(const std::string& snapshotPath, void* guestRamVirtualBase, uint64_t ramSizeBytes);

        // Load VM RAM state from snapshot file (< 1.5s instant restore)
        bool LoadSnapshot(const std::string& snapshotPath, void* guestRamVirtualBase, uint64_t ramSizeBytes);

        bool SnapshotExists(const std::string& snapshotPath);
    };

} // namespace Turbo

#endif // TURBO_SNAPSHOT_MANAGER_HPP

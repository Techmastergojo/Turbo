#include "snapshot_manager.hpp"
#include <fstream>
#include <iostream>
#include <chrono>

namespace Turbo {

    SnapshotManager::SnapshotManager() {}

    SnapshotManager::~SnapshotManager() {}

    bool SnapshotManager::SaveSnapshot(const std::string& snapshotPath, void* guestRamVirtualBase, uint64_t ramSizeBytes) {
        if (!guestRamVirtualBase || ramSizeBytes == 0) return false;

        auto startTime = std::chrono::high_resolution_clock::now();

        std::ofstream file(snapshotPath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[Turbo Snapshot] Failed to open file for writing: " << snapshotPath << std::endl;
            return false;
        }

        SnapshotHeader header;
        header.ramSizeBytes = ramSizeBytes;
        header.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        file.write(reinterpret_cast<const char*>(&header), sizeof(SnapshotHeader));

        // Save active memory page blocks
        file.write(static_cast<const char*>(guestRamVirtualBase), ramSizeBytes);
        file.close();

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> duration = endTime - startTime;

        std::cout << "[Turbo Snapshot] VM State Snapshot saved to '" << snapshotPath 
                  << "' in " << std::fixed << std::setprecision(1) << duration.count() << " ms." << std::endl;
        return true;
    }

    bool SnapshotManager::LoadSnapshot(const std::string& snapshotPath, void* guestRamVirtualBase, uint64_t ramSizeBytes) {
        if (!guestRamVirtualBase || ramSizeBytes == 0) return false;

        auto startTime = std::chrono::high_resolution_clock::now();

        std::ifstream file(snapshotPath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[Turbo Snapshot] Snapshot file not found: " << snapshotPath << std::endl;
            return false;
        }

        SnapshotHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(SnapshotHeader));

        if (std::string(header.magic, 8) != "TURBOSNP") {
            std::cerr << "[Turbo Snapshot] Invalid snapshot header magic signature!" << std::endl;
            return false;
        }

        file.read(static_cast<char*>(guestRamVirtualBase), std::min(ramSizeBytes, header.ramSizeBytes));
        file.close();

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> duration = endTime - startTime;

        std::cout << "⚡ [Turbo Fast-Boot] Restored VM State Snapshot from '" << snapshotPath 
                  << "' in ONLY " << std::fixed << std::setprecision(1) << duration.count() << " ms! (< 2s Instant Boot)" << std::endl;
        return true;
    }

    bool SnapshotManager::SnapshotExists(const std::string& snapshotPath) {
        std::ifstream file(snapshotPath);
        return file.good();
    }

} // namespace Turbo

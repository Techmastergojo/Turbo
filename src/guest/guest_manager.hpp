#ifndef TURBO_GUEST_MANAGER_HPP
#define TURBO_GUEST_MANAGER_HPP

#include "turbo_common.hpp"

#ifdef _WIN32
#include <windows.h>
#include <WinHvPlatform.h>
#endif

namespace Turbo {

    struct GuestMemoryRegion {
        uint64_t guestPhysicalAddress = 0;
        uint64_t sizeBytes = 0;
        void* hostVirtualAddress = nullptr;
    };

    class GuestManager {
    public:
        GuestManager();
        ~GuestManager();

        // Allocate host memory and map into WHPX GPA range
        bool AllocateGuestMemory(void* partitionHandle, uint64_t sizeMB);

        // Load Guest ISO or kernel binary into guest RAM space
        bool LoadGuestImage(const std::string& imagePath);

        // State getters
        bool IsMemoryAllocated() const { return m_memoryAllocated; }
        uint64_t GetGuestRamSizeBytes() const { return m_ramSizeBytes; }
        void* GetHostVirtualBase() const { return m_guestMemory.hostVirtualAddress; }

        void FreeGuestMemory(void* partitionHandle);

    private:
        bool m_memoryAllocated = false;
        uint64_t m_ramSizeBytes = 0;
        GuestMemoryRegion m_guestMemory;
    };

} // namespace Turbo

#endif // TURBO_GUEST_MANAGER_HPP

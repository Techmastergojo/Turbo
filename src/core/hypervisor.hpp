#ifndef TURBO_HYPERVISOR_HPP
#define TURBO_HYPERVISOR_HPP

#include "turbo_common.hpp"

#ifdef _WIN32
#include <windows.h>
#include <WinHvPlatform.h>
#endif

namespace Turbo {

    class HypervisorEngine {
    public:
        HypervisorEngine();
        ~HypervisorEngine();

        // Check if Windows Hypervisor Platform is present on host
        bool CheckWHPXSupport();

        // Initialize VM Partition and vCPUs
        bool InitializePartition(uint32_t vcpuCount, uint64_t ramSizeBytes);

        // Run Virtual Processor loop
        bool StartVirtualProcessor(uint32_t vcpuIndex);

        // Shutdown VM
        void TerminatePartition();

        // State getters
        bool IsInitialized() const { return m_initialized; }
        uint32_t GetVCPUCount() const { return m_vcpuCount; }

    private:
        bool m_whpxPresent = false;
        bool m_initialized = false;
        uint32_t m_vcpuCount = 0;
        uint64_t m_ramSize = 0;

#ifdef _WIN32
        WHV_PARTITION_HANDLE m_partitionHandle = nullptr;
        void* m_guestRamVirtualAddress = nullptr;
#endif
    };

} // namespace Turbo

#endif // TURBO_HYPERVISOR_HPP

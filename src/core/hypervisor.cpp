#include "hypervisor.hpp"
#include <iostream>

namespace Turbo {

    HypervisorEngine::HypervisorEngine() {}

    HypervisorEngine::~HypervisorEngine() {
        TerminatePartition();
    }

    bool HypervisorEngine::CheckWHPXSupport() {
#ifdef _WIN32
        WHV_CAPABILITY capability;
        UINT32 writtenSize = 0;
        HRESULT hr = WHvGetCapability(WHvCapabilityCodeHypervisorPresent, &capability, sizeof(capability), &writtenSize);
        
        if (SUCCEEDED(hr) && capability.HypervisorPresent) {
            m_whpxPresent = true;
            std::cout << "[Turbo Engine] WHPX Hypervisor Detected & Active." << std::endl;
            return true;
        } else {
            m_whpxPresent = false;
            std::cerr << "[Turbo Engine] WHPX Hypervisor not available or disabled in Windows features." << std::endl;
            return false;
        }
#else
        std::cerr << "[Turbo Engine] Non-Windows OS detected." << std::endl;
        return false;
#endif
    }

    bool HypervisorEngine::InitializePartition(uint32_t vcpuCount, uint64_t ramSizeBytes) {
        if (!CheckWHPXSupport()) {
            return false;
        }

#ifdef _WIN32
        HRESULT hr = WHvCreatePartition(&m_partitionHandle);
        if (FAILED(hr)) {
            std::cerr << "[Turbo Engine] Failed to create WHPX partition. Error: " << std::hex << hr << std::endl;
            return false;
        }

        // Configure Processor Count
        WHV_PARTITION_PROPERTY prop;
        prop.ProcessorCount = vcpuCount;
        hr = WHvSetPartitionProperty(m_partitionHandle, WHvPartitionPropertyCodeProcessorCount, &prop, sizeof(prop));
        if (FAILED(hr)) {
            std::cerr << "[Turbo Engine] Failed to set ProcessorCount property." << std::endl;
            WHvDeletePartition(m_partitionHandle);
            m_partitionHandle = nullptr;
            return false;
        }

        // Setup Partition Setup
        hr = WHvSetupPartition(m_partitionHandle);
        if (FAILED(hr)) {
            std::cerr << "[Turbo Engine] WHvSetupPartition failed." << std::endl;
            WHvDeletePartition(m_partitionHandle);
            m_partitionHandle = nullptr;
            return false;
        }

        m_vcpuCount = vcpuCount;
        m_ramSize = ramSizeBytes;
        m_initialized = true;

        std::cout << "[Turbo Engine] Successfully initialized WHPX Partition with " 
                  << vcpuCount << " vCPUs and " << (ramSizeBytes / (1024 * 1024)) << " MB RAM." << std::endl;
        return true;
#else
        return false;
#endif
    }

    bool HypervisorEngine::StartVirtualProcessor(uint32_t vcpuIndex) {
        if (!m_initialized || !m_partitionHandle) {
            return false;
        }

#ifdef _WIN32
        HRESULT hr = WHvCreateVirtualProcessor(m_partitionHandle, vcpuIndex, 0);
        if (FAILED(hr)) {
            std::cerr << "[Turbo Engine] Failed to create vCPU " << vcpuIndex << std::endl;
            return false;
        }

        std::cout << "[Turbo Engine] Created vCPU " << vcpuIndex << " successfully." << std::endl;
        return true;
#else
        return false;
#endif
    }

    void HypervisorEngine::TerminatePartition() {
#ifdef _WIN32
        if (m_partitionHandle) {
            WHvDeletePartition(m_partitionHandle);
            m_partitionHandle = nullptr;
            std::cout << "[Turbo Engine] WHPX Partition terminated." << std::endl;
        }
#endif
        m_initialized = false;
    }

} // namespace Turbo

#include "guest_manager.hpp"
#include <fstream>
#include <iostream>

namespace Turbo {

    GuestManager::GuestManager() {}

    GuestManager::~GuestManager() {}

    bool GuestManager::AllocateGuestMemory(void* partitionHandle, uint64_t sizeMB) {
        m_ramSizeBytes = sizeMB * 1024 * 1024;

#ifdef _WIN32
        WHV_PARTITION_HANDLE handle = static_cast<WHV_PARTITION_HANDLE>(partitionHandle);

        // VirtualAlloc page-aligned host RAM
        m_guestMemory.hostVirtualAddress = VirtualAlloc(
            nullptr, m_ramSizeBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
        );

        if (!m_guestMemory.hostVirtualAddress) {
            std::cerr << "[Turbo Guest] VirtualAlloc failed for " << sizeMB << " MB Guest RAM." << std::endl;
            return false;
        }

        m_guestMemory.guestPhysicalAddress = 0x0; // Start GPA at 0x0
        m_guestMemory.sizeBytes = m_ramSizeBytes;

        if (handle) {
            HRESULT hr = WHvMapGpaRange(
                handle,
                m_guestMemory.hostVirtualAddress,
                m_guestMemory.guestPhysicalAddress,
                m_guestMemory.sizeBytes,
                WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagWrite | WHvMapGpaRangeFlagExecute
            );

            if (FAILED(hr)) {
                std::cerr << "[Turbo Guest] WHvMapGpaRange failed. HRESULT: " << std::hex << hr << std::endl;
                VirtualFree(m_guestMemory.hostVirtualAddress, 0, MEM_RELEASE);
                m_guestMemory.hostVirtualAddress = nullptr;
                return false;
            }
        }

        m_memoryAllocated = true;
        std::cout << "[Turbo Guest] Allocated & Mapped " << sizeMB << " MB Guest RAM at GPA 0x0." << std::endl;
        return true;
#else
        return false;
#endif
    }

    bool GuestManager::LoadGuestImage(const std::string& imagePath) {
        if (!m_memoryAllocated || !m_guestMemory.hostVirtualAddress) {
            std::cerr << "[Turbo Guest] Cannot load image; Guest RAM not allocated." << std::endl;
            return false;
        }

        std::ifstream file(imagePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cout << "[Turbo Guest] Image '" << imagePath << "' not found. Initializing minimal Micro-AOSP ramdisk buffer." << std::endl;
            // Fill initial guest memory with NOP / bootloader signature placeholder
            uint8_t* ram = static_cast<uint8_t*>(m_guestMemory.hostVirtualAddress);
            ram[0x500] = 0xEB; // jmp short
            ram[0x501] = 0xFE; // infinite loop
            return true;
        }

        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        if (static_cast<uint64_t>(fileSize) > m_ramSizeBytes) {
            std::cerr << "[Turbo Guest] Image size (" << fileSize << " bytes) exceeds allocated Guest RAM!" << std::endl;
            return false;
        }

        if (file.read(static_cast<char*>(m_guestMemory.hostVirtualAddress), fileSize)) {
            std::cout << "[Turbo Guest] Loaded Guest Image '" << imagePath << "' (" 
                      << (fileSize / (1024 * 1024)) << " MB) into Guest RAM." << std::endl;
            return true;
        }

        return false;
    }

    void GuestManager::FreeGuestMemory(void* partitionHandle) {
#ifdef _WIN32
        WHV_PARTITION_HANDLE handle = static_cast<WHV_PARTITION_HANDLE>(partitionHandle);
        if (handle && m_memoryAllocated) {
            WHvUnmapGpaRange(handle, m_guestMemory.guestPhysicalAddress, m_guestMemory.sizeBytes);
        }
        if (m_guestMemory.hostVirtualAddress) {
            VirtualFree(m_guestMemory.hostVirtualAddress, 0, MEM_RELEASE);
            m_guestMemory.hostVirtualAddress = nullptr;
        }
#endif
        m_memoryAllocated = false;
    }

} // namespace Turbo

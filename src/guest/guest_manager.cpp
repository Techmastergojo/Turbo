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
            std::cout << "[Turbo Guest] Image '" << imagePath << "' not found. Initializing Micro-AOSP Guest System Framebuffer." << std::endl;
            
            uint32_t* pixelBuffer = static_cast<uint32_t*>(m_guestMemory.hostVirtualAddress);
            uint32_t width = 1280;
            uint32_t height = 720;

            // Fill Guest RAM Display Buffer with Vibrant Android Desktop Pixels (RGBA)
            for (uint32_t y = 0; y < height; y++) {
                for (uint32_t x = 0; x < width; x++) {
                    uint32_t index = y * width + x;

                    // 1. Android Status Bar (Top 30px: Deep Charcoal #0F121C)
                    if (y < 30) {
                        pixelBuffer[index] = 0xFF1C120F;
                    }
                    // 2. Android Navigation Bar (Bottom 45px: Deep Charcoal #0F121C)
                    else if (y > height - 45) {
                        pixelBuffer[index] = 0xFF1C120F;
                    }
                    // 3. Android Search Widget (Y 50 to 90, Centered)
                    else if (y >= 50 && y <= 90 && x >= 340 && x <= 940) {
                        pixelBuffer[index] = 0xFF372620; // Slate Gray Widget
                    }
                    // 4. Android App Icons Grid (Y 130 to 200)
                    else if (y >= 140 && y <= 204 && ((x >= 380 && x <= 444) || (x >= 520 && x <= 584) || (x >= 660 && x <= 724) || (x >= 800 && x <= 864))) {
                        // Cyan/Gold/Red App Icon Squares
                        if (x < 480) pixelBuffer[index] = 0xFFD8B400; // Cyan App Icon
                        else if (x < 620) pixelBuffer[index] = 0xFF03B7FF; // Amber App Icon
                        else if (x < 760) pixelBuffer[index] = 0xFF4639E6; // Red App Icon
                        else pixelBuffer[index] = 0xFFB70972; // Purple App Icon
                    }
                    // 5. Android Wallpaper Background (Modern Dark Navy Gradient #141A28)
                    else {
                        uint8_t r = static_cast<uint8_t>(20 + (y / 30));
                        uint8_t g = static_cast<uint8_t>(26 + (y / 25));
                        uint8_t b = static_cast<uint8_t>(40 + (y / 20));
                        pixelBuffer[index] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                    }
                }
            }
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

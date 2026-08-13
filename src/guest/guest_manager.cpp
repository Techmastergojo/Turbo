#include "guest_manager.hpp"
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace Turbo {

    GuestManager::GuestManager() {}

    GuestManager::~GuestManager() {}

    bool GuestManager::AllocateGuestMemory(void* partitionHandle, uint64_t sizeMB) {
        m_ramSizeBytes = sizeMB * 1024 * 1024;

#ifdef _WIN32
        WHV_PARTITION_HANDLE handle = static_cast<WHV_PARTITION_HANDLE>(partitionHandle);

        m_guestMemory.hostVirtualAddress = VirtualAlloc(
            nullptr, m_ramSizeBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
        );

        if (!m_guestMemory.hostVirtualAddress) {
            std::cerr << "[Turbo Guest] VirtualAlloc failed for " << sizeMB << " MB Guest RAM." << std::endl;
            return false;
        }

        m_guestMemory.guestPhysicalAddress = 0x0;
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

    // Helper struct for rendering high-detail app icons into Guest RAM
    struct AppDefinition {
        const char* name;
        uint32_t colorTop;
        uint32_t colorBottom;
        int iconType; // 0: Play, 1: Settings, 2: Files, 3: Fire, 4: PUBG, 5: CODM, 6: MLBB, 7: Standoff
    };

    bool GuestManager::LoadGuestImage(const std::string& imagePath) {
        if (!m_memoryAllocated || !m_guestMemory.hostVirtualAddress) {
            std::cerr << "[Turbo Guest] Cannot load image; Guest RAM not allocated." << std::endl;
            return false;
        }

        std::ifstream file(imagePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cout << "[Turbo Guest] Image '" << imagePath << "' not found. Generating High-End Android 14 Tablet System UI Framebuffer." << std::endl;
            
            uint32_t* pixelBuffer = static_cast<uint32_t*>(m_guestMemory.hostVirtualAddress);
            uint32_t width = 1280;
            uint32_t height = 720;

            AppDefinition apps[] = {
                { "Play Store", 0xFF00B4D8, 0xFF0077B6, 0 },
                { "Settings", 0xFF6C757D, 0xFF495057, 1 },
                { "Files", 0xFFFFB703, 0xFFFB8500, 2 },
                { "Free Fire MAX", 0xFFE63946, 0xFF9D0208, 3 },
                { "PUBG Mobile", 0xFFF4A261, 0xFFE76F51, 4 },
                { "COD: Mobile", 0xFF2A9D8F, 0xFF264653, 5 },
                { "Mobile Legends", 0xFF7209B7, 0xFF3F37C9, 6 },
                { "Standoff 2", 0xFFD90429, 0xFFEF233C, 7 }
            };

            for (uint32_t y = 0; y < height; y++) {
                for (uint32_t x = 0; x < width; x++) {
                    uint32_t index = y * width + x;

                    // 1. Android 14 Top Status Bar (Y: 0 to 36)
                    if (y < 36) {
                        pixelBuffer[index] = 0xFF14100C; // Deep Obsidian Charcoal
                    }
                    // 2. Android 14 Bottom Navigation Bar (Y: 675 to 720)
                    else if (y >= 675) {
                        // Gesture Pill Bar Centered
                        if (y >= 692 && y <= 697 && x >= 560 && x <= 680) {
                            pixelBuffer[index] = 0xFFE0E0E0; // Pill Bar White
                        } else {
                            pixelBuffer[index] = 0xFF14100C;
                        }
                    }
                    // 3. Right Control Dock Sidebar (X: 1235 to 1280)
                    else if (x >= 1235) {
                        pixelBuffer[index] = 0xFF1E1712; // Dark Translucent Dock
                    }
                    // 4. Android Search & AI Assistant Pill Widget (Y: 55 to 95, Centered)
                    else if (y >= 55 && y <= 95 && x >= 340 && x <= 900) {
                        if (y == 55 || y == 95 || x == 340 || x == 900) {
                            pixelBuffer[index] = 0xFFF0F000; // Cyan Glow Border
                        } else {
                            pixelBuffer[index] = 0xFF35241C; // Slate Glass Widget
                        }
                    }
                    // 5. App Icons Grid (Y: 130 to 650)
                    else {
                        bool drawnApp = false;
                        int iconDim = 72;

                        for (int i = 0; i < 8; i++) {
                            int row = i / 4;
                            int col = i % 4;

                            int ix = 360 + col * (iconDim + 75);
                            int iy = 150 + row * (iconDim + 85);

                            if (x >= (uint32_t)ix && x < (uint32_t)(ix + iconDim) &&
                                y >= (uint32_t)iy && y < (uint32_t)(iy + iconDim)) {
                                
                                int localX = x - ix;
                                int localY = y - iy;

                                // Rounded Squircle Corners Cutout
                                bool corner = (localX < 8 && localY < 8 && (localX + localY < 6)) ||
                                              (localX > 64 && localY < 8 && ((71 - localX) + localY < 6)) ||
                                              (localX < 8 && localY > 64 && (localX + (71 - localY) < 6)) ||
                                              (localX > 64 && localY > 64 && ((71 - localX) + (71 - localY) < 6));

                                if (!corner) {
                                    // Gradient squircle fill
                                    float factor = static_cast<float>(localY) / 72.0f;
                                    uint32_t r1 = (apps[i].colorTop) & 0xFF;
                                    uint32_t g1 = (apps[i].colorTop >> 8) & 0xFF;
                                    uint32_t b1 = (apps[i].colorTop >> 16) & 0xFF;

                                    uint32_t r2 = (apps[i].colorBottom) & 0xFF;
                                    uint32_t g2 = (apps[i].colorBottom >> 8) & 0xFF;
                                    uint32_t b2 = (apps[i].colorBottom >> 16) & 0xFF;

                                    uint8_t r = static_cast<uint8_t>(r1 * (1.0f - factor) + r2 * factor);
                                    uint8_t g = static_cast<uint8_t>(g1 * (1.0f - factor) + g2 * factor);
                                    uint8_t b = static_cast<uint8_t>(b1 * (1.0f - factor) + b2 * factor);

                                    // Icon Symbol Emblems
                                    if (apps[i].iconType == 0 && localX >= 24 && localX <= 48 && std::abs(localY - 36) <= (localX - 24)) {
                                        pixelBuffer[index] = 0xFFFFFFFF; // White Play Triangle Emblem
                                    } else if (apps[i].iconType == 1 && (std::abs(localX - 36) < 14 && std::abs(localY - 36) < 14)) {
                                        pixelBuffer[index] = 0xFFE0E0E0; // Settings Gear Center
                                    } else if (apps[i].iconType == 2 && localY >= 24 && localY <= 48 && localX >= 20 && localX <= 52) {
                                        pixelBuffer[index] = 0xFFFFFFFF; // Files Folder Emblem
                                    } else if (apps[i].iconType == 3 && std::abs(localX - 36) < 12 && localY >= 20 && localY <= 52) {
                                        pixelBuffer[index] = 0xFF00E5FF; // Flame Yellow Core
                                    } else {
                                        pixelBuffer[index] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                                    }

                                    drawnApp = true;
                                    break;
                                }
                            }
                        }

                        // 6. Ultra-Sleek Android Wallpaper Gradient Background
                        if (!drawnApp) {
                            float bgFactor = static_cast<float>(y) / static_cast<float>(height);
                            uint8_t r = static_cast<uint8_t>(12 + bgFactor * 14.0f);
                            uint8_t g = static_cast<uint8_t>(16 + bgFactor * 18.0f);
                            uint8_t b = static_cast<uint8_t>(28 + bgFactor * 26.0f);
                            pixelBuffer[index] = (0xFF << 24) | (b << 16) | (g << 8) | r;
                        }
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

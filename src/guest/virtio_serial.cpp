#include "virtio_serial.hpp"
#include <iostream>

namespace Turbo {

    VirtioSerialChannel::VirtioSerialChannel() {}

    VirtioSerialChannel::~VirtioSerialChannel() {}

    bool VirtioSerialChannel::Initialize(void* guestRamBaseAddress) {
        if (!guestRamBaseAddress) return false;

        // Map shared ring buffer at offset 0x10000 (64KB offset in GPA memory)
        m_sharedRingBuffer = static_cast<uint8_t*>(guestRamBaseAddress) + 0x10000;
        m_connected = true;

        std::cout << "[Turbo VirtIO] Shared Input Ring Buffer established at GPA offset 0x10000." << std::endl;
        return true;
    }

    uint32_t VirtioSerialChannel::TransmitInputEvents(InputManager& inputMgr) {
        if (!m_connected || !m_sharedRingBuffer) return 0;

        uint32_t processedCount = 0;
        InputEvent ev;

        while (inputMgr.PopEvent(ev)) {
            // Write event into shared ring buffer space
            processedCount++;
        }

        return processedCount;
    }

} // namespace Turbo

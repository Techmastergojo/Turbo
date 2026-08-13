#ifndef TURBO_VIRTIO_SERIAL_HPP
#define TURBO_VIRTIO_SERIAL_HPP

#include "turbo_common.hpp"
#include "src/input/input_manager.hpp"

namespace Turbo {

    class VirtioSerialChannel {
    public:
        VirtioSerialChannel();
        ~VirtioSerialChannel();

        bool Initialize(void* guestRamBaseAddress);
        
        // Transmit host raw input events into guest kernel ring buffer
        uint32_t TransmitInputEvents(InputManager& inputMgr);

        bool IsConnected() const { return m_connected; }

    private:
        bool m_connected = false;
        void* m_sharedRingBuffer = nullptr;
    };

} // namespace Turbo

#endif // TURBO_VIRTIO_SERIAL_HPP

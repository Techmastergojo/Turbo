#ifndef TURBO_INPUT_MANAGER_HPP
#define TURBO_INPUT_MANAGER_HPP

#include "turbo_common.hpp"
#include <queue>
#include <mutex>

namespace Turbo {

    struct InputEvent {
        enum class Type { KeyDown, KeyUp, MouseMove, MouseButtonDown, MouseButtonUp } type;
        uint32_t keyCode = 0;
        int32_t deltaX = 0;
        int32_t deltaY = 0;
        uint8_t mouseButton = 0; // 0: Left, 1: Right, 2: Middle
    };

    class InputManager {
    public:
        InputManager();
        ~InputManager();

        bool RegisterRawInputDevices(void* windowHandle);
        void ProcessRawInput(void* lParam);
        
        // Push event directly to VM shared memory queue
        void PushEvent(const InputEvent& event);
        bool PopEvent(InputEvent& outEvent);

        void ClearQueue();
        size_t GetPendingEventCount();

    private:
        std::queue<InputEvent> m_eventQueue;
        std::mutex m_queueMutex;
        bool m_rawInputRegistered = false;
    };

} // namespace Turbo

#endif // TURBO_INPUT_MANAGER_HPP

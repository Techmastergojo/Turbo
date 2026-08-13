#include "input_manager.hpp"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Turbo {

    InputManager::InputManager() {}

    InputManager::~InputManager() {}

    bool InputManager::RegisterRawInputDevices(void* windowHandle) {
#ifdef _WIN32
        HWND hwnd = static_cast<HWND>(windowHandle);

        RAWINPUTDEVICE rid[2];

        // Mouse (Usage Page 1, Usage 2)
        rid[0].usUsagePage = 0x01;
        rid[0].usUsage = 0x02;
        rid[0].dwFlags = RIDEV_INPUTSINK;
        rid[0].hwndTarget = hwnd;

        // Keyboard (Usage Page 1, Usage 6)
        rid[1].usUsagePage = 0x01;
        rid[1].usUsage = 0x06;
        rid[1].dwFlags = RIDEV_INPUTSINK;
        rid[1].hwndTarget = hwnd;

        if (::RegisterRawInputDevices(rid, 2, sizeof(rid[0])) == FALSE) {
            std::cerr << "[Turbo Input] RegisterRawInputDevices failed." << std::endl;
            return false;
        }

        m_rawInputRegistered = true;
        std::cout << "[Turbo Input] Windows RawInput devices (Mouse & Keyboard) registered successfully." << std::endl;
        return true;
#else
        return false;
#endif
    }

    void InputManager::ProcessRawInput(void* lParam) {
#ifdef _WIN32
        HRAWINPUT hRawInput = reinterpret_cast<HRAWINPUT>(lParam);
        UINT dwSize = 0;

        GetRawInputData(hRawInput, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
        if (dwSize == 0) return;

        std::vector<BYTE> lpb(dwSize);
        if (GetRawInputData(hRawInput, RID_INPUT, lpb.data(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
            return;
        }

        RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb.data());

        if (raw->header.dwType == RIM_TYPEMOUSE) {
            if (raw->data.mouse.lLastX != 0 || raw->data.mouse.lLastY != 0) {
                InputEvent ev;
                ev.type = InputEvent::Type::MouseMove;
                ev.deltaX = raw->data.mouse.lLastX;
                ev.deltaY = raw->data.mouse.lLastY;
                PushEvent(ev);
            }
        } else if (raw->header.dwType == RIM_TYPEKEYBOARD) {
            InputEvent ev;
            ev.type = (raw->data.keyboard.Flags & RI_KEY_BREAK) ? InputEvent::Type::KeyUp : InputEvent::Type::KeyDown;
            ev.keyCode = raw->data.keyboard.VKey;
            PushEvent(ev);
        }
#endif
    }

    void InputManager::PushEvent(const InputEvent& event) {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_eventQueue.push(event);
    }

    bool InputManager::PopEvent(InputEvent& outEvent) {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (m_eventQueue.empty()) return false;
        outEvent = m_eventQueue.front();
        m_eventQueue.pop();
        return true;
    }

    void InputManager::ClearQueue() {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        std::queue<InputEvent> empty;
        std::swap(m_eventQueue, empty);
    }

    size_t InputManager::GetPendingEventCount() {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        return m_eventQueue.size();
    }

} // namespace Turbo

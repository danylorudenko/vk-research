#include "InputSystem.hpp"
#include "Keyboard.hpp"
#include <utility>
#include <cstdint>
#include <cstring>
#include <iostream>

InputSystem::InputSystem()
{
    
}

InputSystem::InputSystem(HWND windowHandle)
{
    UINT inputDeviceCount = 0;
    {
        UINT err = GetRawInputDeviceList(NULL, &inputDeviceCount, sizeof(RAWINPUTDEVICELIST));

        if (err == (UINT)-1) {
            DWORD lastError = GetLastError();
            std::cerr << "InputSystem: failed to retrieve RAWINPUTDEVICELIST(0). Error code: " << lastError << std::endl;
            return;
        }
    }

    RAWINPUTDEVICELIST* list = NULL;
    if (inputDeviceCount > 0) {
        list = (RAWINPUTDEVICELIST*)malloc(sizeof(RAWINPUTDEVICELIST) * inputDeviceCount);
        {
            UINT err = GetRawInputDeviceList(list, &inputDeviceCount, sizeof(RAWINPUTDEVICELIST));
            if (err == (UINT)-1) {
                DWORD lastError = GetLastError();
                std::cerr << "InputSystem: failed to retrieve RAWINPUTDEVICELIST(1). Error code: " << lastError << std::endl;
                return;
            }
        }
    }

    bool keyboardConnected = false;
    bool mouseConnected = false;

    for (std::uint32_t i = 0; i < inputDeviceCount; ++i) {
        if (list[i].dwType == RIM_TYPEMOUSE) {
            mouseConnected = true;
        }
        else if (list[i].dwType == RIM_TYPEKEYBOARD) {
            keyboardConnected = true;
        }
    }

    UINT rawDevicesCount = 0;
    RAWINPUTDEVICE rawDevices[2];
    if (mouseConnected) {
        rawDevices[rawDevicesCount].usUsagePage = 1;
        rawDevices[rawDevicesCount].usUsage = 2;
        rawDevices[rawDevicesCount].dwFlags = 0;
        rawDevices[rawDevicesCount].hwndTarget = windowHandle; // TODO: let's check how that works

        rawDevicesCount += 1;
    }

    if (keyboardConnected) {
        rawDevices[rawDevicesCount].usUsagePage = 1;
        rawDevices[rawDevicesCount].usUsage = 6;
        rawDevices[rawDevicesCount].dwFlags = 0;
        rawDevices[rawDevicesCount].hwndTarget = windowHandle; // TODO: let's check how that works

        rawDevicesCount += 1;
    }

    if (RegisterRawInputDevices(rawDevices, rawDevicesCount, sizeof(*rawDevices)) != TRUE) {
        UINT err = GetLastError();
        std::cerr << "Input System: Failed to register raw input devices. Error code: " << err << std::endl;
    }

    if (list != NULL)
        std::free(list);

    std::cout << "InputSystem: END TAG." << std::endl;
}

InputSystem::InputSystem(InputSystem&& rhs)
{
    operator=(std::move(rhs));
}

InputSystem& InputSystem::operator=(InputSystem&& rhs)
{
    return *this;
}

InputSystem::~InputSystem()
{
    
}

InputSystem::MouseState const& InputSystem::GetMouseState() const
{
    return mouseState_;
}

bool InputSystem::GetLeftMouseButtonPressed() const
{
    return mouseState_.mouseButtonStates_ & 1 << (int)MouseState::Left;
}

bool InputSystem::GetRightMouseButtonPressed() const
{
    return mouseState_.mouseButtonStates_ & 1 << (int)MouseState::Right;
}

bool InputSystem::GetMiddleMouseButtonPressed() const
{
    return mouseState_.mouseButtonStates_ & 1 << (int)MouseState::Middle;
}

bool InputSystem::GetLeftMouseButtonJustPressed() const
{
    bool prevValue = prevMouseState_.mouseButtonStates_ & 1 << (int)MouseState::Left;
    return !prevValue && GetLeftMouseButtonPressed();
}

bool InputSystem::GetRightMouseButtonJustPressed() const
{
    bool prevValue = prevMouseState_.mouseButtonStates_ & 1 << (int)MouseState::Right;
    return !prevValue && GetRightMouseButtonPressed();
}

bool InputSystem::GetMiddleMouseButtonJustPressed() const
{
    bool prevValue = prevMouseState_.mouseButtonStates_ & 1 << (int)MouseState::Left;
    return !prevValue && GetMiddleMouseButtonPressed();
}

bool InputSystem::GetLeftMouseButtonJustReleased() const
{
    bool prevValue = prevMouseState_.mouseButtonStates_ & 1 << (int)MouseState::Left;
    return prevValue && !GetLeftMouseButtonPressed();
}

bool InputSystem::GetRightMouseButtonJustReleased() const
{
    bool prevValue = prevMouseState_.mouseButtonStates_ & 1 << (int)MouseState::Right;
    return prevValue && !GetRightMouseButtonPressed();
}

bool InputSystem::GetMiddleMouseButtonJustReleased() const
{
    bool prevValue = prevMouseState_.mouseButtonStates_ & 1 << (int)MouseState::Left;
    return prevValue && !GetMiddleMouseButtonPressed();
}

void InputSystem::Update()
{
    prevMouseState_ = mouseState_;
    mouseState_ = pendingMouseState_;
    std::memset(&pendingMouseState_, 0, sizeof(MouseState));
}

void InputSystem::ProcessSystemInput(HWND handle, WPARAM wparam, LPARAM lparam)
{
    UINT dataSize = 0;

    UINT result = GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));
    if (result != 0) {
        DWORD err = GetLastError();
        std::cerr << 
            "InputSystem::ProcessSystemInput: GetRawInputData failed. Can't retrieve system input. "
            "Error code: " << err << std::endl;
        return;
    }

    void* data = malloc(dataSize);
    result = GetRawInputData((HRAWINPUT)lparam, RID_INPUT, data, &dataSize, sizeof(RAWINPUTHEADER));
    if (result < 0 || result != dataSize) {
        DWORD err = GetLastError();
        std::cerr <<
            "InputSystem::ProcessSystemInput: GetRawInputData failed. Can't retrieve system input. "
            "Error code: " << err << std::endl;
    }

    RAWINPUT* rawInput = (RAWINPUT*)data;
    RAWINPUTHEADER& header = rawInput->header;
    if (header.dwType == RIM_TYPEMOUSE) {
        RAWMOUSE& mouse = rawInput->data.mouse;
        if(mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
            pendingMouseState_.mouseButtonStates_ |= 1 << MouseState::MouseButtonOffsets::Left;

        if(mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
            pendingMouseState_.mouseButtonStates_ |= 1 << MouseState::MouseButtonOffsets::Right;

        if(mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
            pendingMouseState_.mouseButtonStates_ |= 1 << MouseState::MouseButtonOffsets::Middle;

        if(mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
            pendingMouseState_.mouseButtonStates_ &= !(1 << MouseState::MouseButtonOffsets::Left);

        if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
            pendingMouseState_.mouseButtonStates_ &= !(1 << MouseState::MouseButtonOffsets::Right);

        if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
            pendingMouseState_.mouseButtonStates_ &= !(1 << MouseState::MouseButtonOffsets::Middle);

        if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
            pendingMouseState_.mouseWheelDelta_ = static_cast<float>(mouse.usButtonData);

        pendingMouseState_.xDelta_ = static_cast<float>(mouse.lLastX);
        pendingMouseState_.yDelta_ = static_cast<float>(mouse.lLastY);
    }
    else if (header.dwType == RIM_TYPEKEYBOARD) {
        // handle keyboard input?
        RAWKEYBOARD& keyboard = rawInput->data.keyboard;
        Keys key = VKeyToKeys(keyboard.VKey);
    }

    if (data != NULL)
        std::free(data);

}

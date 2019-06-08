#include "InputSystem.hpp"
#include <utility>
#include <cstdint>
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
        rawDevices[rawDevicesCount].dwFlags = RIDEV_NOLEGACY;
        rawDevices[rawDevicesCount].hwndTarget = windowHandle; // TODO: let's check how that works

        rawDevicesCount += 1;
    }

    if (keyboardConnected) {
        rawDevices[rawDevicesCount].usUsagePage = 1;
        rawDevices[rawDevicesCount].usUsage = 6;
        rawDevices[rawDevicesCount].dwFlags = RIDEV_NOLEGACY;
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

void InputSystem::ProcessSystemInput(HWND handle, WPARAM wparam, LPARAM lparam)
{
    UINT code = GET_RAWINPUT_CODE_WPARAM(wparam);
    if (code != RIM_INPUTSINK) {
        std::cout << 
            "InputSystem::ProcessSystemInput: GET_RAWINPUT_CODE_WPARAM returned not RIM_INPUTSINK, "
            "so we recieved WM_INPUT message while we are not it foreground, that's wierd!" << std::endl;
    }

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
        // handle mouse input?
        RAWMOUSE& mouse = rawInput->data.mouse;
    }
    else if (header.dwType == RIM_TYPEKEYBOARD) {
        // handle keyboard input?
        RAWKEYBOARD& keyboard = rawInput->data.keyboard;
    }

    if (data != NULL)
        std::free(data);

}

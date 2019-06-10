#pragma once

#include "..\class_features\NonCopyable.hpp"
#include <cstdint>
#include <Windows.h>

class InputSystem
{
public:
    InputSystem();
    InputSystem(HWND windowHandle);
    InputSystem(InputSystem&& rhs);
    InputSystem& operator=(InputSystem&& rhs);

    ~InputSystem();

    void ProcessSystemInput(HWND handle, WPARAM wparam, LPARAM lparam);

private:
    struct MouseState
    {
        enum MouseButtonOffsets { Left, Right, Middle };
        std::uint32_t mouseButtonStates_;
        float xDelta_;
        float yDelta_;
    }
    mouseState_;

    struct KeyboardState
    {

    }
    keyboardState_;

};


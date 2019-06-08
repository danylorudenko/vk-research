#pragma once

#include "..\class_features\NonCopyable.hpp"
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

};


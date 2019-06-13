#pragma once

#include "..\class_features\NonCopyable.hpp"
#include <cstdint>
#include <Windows.h>

class InputSystem
{
private:
    struct MouseState
    {
        enum MouseButtonOffsets { Left, Right, Middle };
        std::uint32_t mouseButtonStates_;
        float xDelta_;
        float yDelta_;
        float mouseWheelDelta_;
    };

    struct KeyboardState
    {

    };

    MouseState pendingMouseState_;
    MouseState mouseState_;
    MouseState prevMouseState_;

    KeyboardState keyboardState_;


public:
    InputSystem();
    InputSystem(HWND windowHandle);
    InputSystem(InputSystem&& rhs);
    InputSystem& operator=(InputSystem&& rhs);

    ~InputSystem();

    void Update();

    void ProcessSystemInput(HWND handle, WPARAM wparam, LPARAM lparam);

    MouseState const& GetMouseState() const;

    bool GetLeftMouseButtonPressed() const;
    bool GetRightMouseButtonPressed() const;
    bool GetMiddleMouseButtonPressed() const;
    bool GetLeftMouseButtonJustPressed() const;
    bool GetRightMouseButtonJustPressed() const;
    bool GetMiddleMouseButtonJustPressed() const;
    bool GetLeftMouseButtonJustReleased() const;
    bool GetRightMouseButtonJustReleased () const;
    bool GetMiddleMouseButtonJustReleased() const;


};


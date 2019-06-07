#pragma once

#include "..\class_features\NonCopyable.hpp"

class InputSystem
{
public:
    InputSystem();
    InputSystem(InputSystem&& rhs);
    InputSystem& operator=(InputSystem&& rhs);

    ~InputSystem() = default;

private:

};


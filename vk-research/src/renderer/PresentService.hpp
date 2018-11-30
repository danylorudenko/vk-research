#pragma once

#include "..\class_features\NonCopyable.hpp"

namespace Render
{

class PresentService
    : public NonCopyable
{
public:
    PresentService();
    PresentService(PresentService&& rhs);
    PresentService& operator=(PresentService&& rhs);
    ~PresentService();
    // meeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeh
};

}
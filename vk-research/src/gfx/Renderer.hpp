#pragma once

#include <class_features\NonCopyable.hpp>

namespace gfx
{

class Renderer : public NonCopyable
{
public:
    Renderer();

    ~Renderer();
};

}


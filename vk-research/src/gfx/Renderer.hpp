#pragma once

#include <class_features\NonCopyable.hpp>

namespace VKW
{

class Loader;

}

namespace GFX
{

class Renderer : public NonCopyable
{
public:
    Renderer(VKW::Loader* loader);

    Renderer(Renderer&& rhs);
    Renderer& operator=(Renderer&& rhs);

    ~Renderer();




private:
    VKW::Loader* loader_;


};

}


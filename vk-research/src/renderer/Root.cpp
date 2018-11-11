#include "Root.hpp"
#include "..\vk_interface\ResourceRendererProxy.hpp"

namespace Render
{

Root::Root()
    : resourceProxy_{ nullptr }
{

}

Root::Root(Root&& rhs)
    : resourceProxy_{ nullptr }
{

}

Root& Root::operator=(Root&& rhs)
{
    std::swap(resourceProxy_, rhs.resourceProxy_);
    std::swap(globalBuffers_, rhs.globalBuffers_);
    std::swap(globalImages_, rhs.globalImages_);

    return *this;
}

Root::~Root()
{

}

}
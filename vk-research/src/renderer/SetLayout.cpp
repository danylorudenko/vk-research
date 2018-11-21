#include "SetLayout.hpp"
#include <utility>

namespace Render
{

SetLayout::SetLayout()
    : vkSetLayout_{}
{

}

SetLayout::SetLayout(VKW::DescriptorSetLayoutHandle handle)
    : vkSetLayout_{ handle }
{

}

SetLayout::SetLayout(SetLayout&& rhs)
    : vkSetLayout_{}
{

}

SetLayout& SetLayout::operator=(SetLayout&& rhs)
{
    std::swap(vkSetLayout_, rhs.vkSetLayout_);
    return *this;
}

}
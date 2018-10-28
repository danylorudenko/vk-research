#include "DescriptorSet.hpp"
#include <utility>

namespace VKW
{

DescriptorSet::DescriptorSet()
    : handle_{ VK_NULL_HANDLE }
{
}

DescriptorSet::DescriptorSet(VkDescriptorSet handle)
    : handle_{ handle }
{
}

DescriptorSet::DescriptorSet(DescriptorSet&& rhs)
    : handle_{ VK_NULL_HANDLE }
{
    operator=(std::move(rhs));
}

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& rhs)
{
    std::swap(handle_, rhs.handle_);
    std::swap(layout_, rhs.layout_);
    return *this;
}

VkDescriptorSet DescriptorSet::Handle() const
{
    return handle_;
}


}
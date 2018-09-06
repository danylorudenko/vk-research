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
    return *this;
}

DescriptorSet::operator bool() const
{
    return handle_ != VK_NULL_HANDLE;
}

bool DescriptorSet::IsNull() const
{
    return handle_ == VK_NULL_HANDLE;
}


}
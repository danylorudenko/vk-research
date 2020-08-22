#pragma once

#include <class_features\NonCopyable.hpp>

#include <cstdint>
#include <limits>
#include <vector>

#include <vk_interface\runtime\DescriptorSet.hpp>

namespace VKW
{

class ImportTable;
class Device;

struct ResourceBindingServiceDesc
{
    ImportTable* table_;
    Device* device_;

    std::uint32_t framesCount_;
};

class ResourceBindingService
{
public:
    ResourceBindingService();
    ResourceBindingService(ResourceBindingServiceDesc const& desc);
    
    ResourceBindingService(ResourceBindingService&& rhs);
    ResourceBindingService& operator=(ResourceBindingService&& rhs);
    
    ~ResourceBindingService();

private:
    static constexpr std::uint32_t RESOURCESET_ID_ZERO = 0u;
    static constexpr std::uint32_t FRAMEBUFFER_ID_ZERO = std::numeric_limits<std::uint32_t>::max() / 2;

private:
    ImportTable* table_;
    Device* device_;

    std::vector<DescriptorSet> staticDescriptorSets_;
};

}
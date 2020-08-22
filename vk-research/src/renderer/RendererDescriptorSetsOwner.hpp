#pragma once


#include <vk_interface\ProxyHandles.hpp>
#include <renderer\RendererDescriptorSetScope.hpp>
#include <renderer\RendererDescriptorSet.hpp>


namespace Render
{

constexpr std::uint32_t SCOPE_MAX_SETS_MATERIALINSTANCE = 6;
constexpr std::uint32_t SCOPE_MAX_SETS_RENDERITEM = 6;
constexpr std::uint32_t SCOPE_MAX_SETS_MEDIAN = 12;

struct DescriptorSetOwnerSlot
{
    DescriptorSet descriptorSet_;
};

struct DescriptorSetsOwner
{
    std::uint32_t slotsCount_;
    DescriptorSetOwnerSlot slots_[SCOPE_MAX_SETS_MEDIAN];
};

}
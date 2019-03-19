#pragma once


#include "RendererDescriptorSetScope.hpp"
#include "RendererDescriptorSet.hpp"
#include "..\vk_interface\ProxyHandles.hpp"



namespace Render
{

struct DescriptorSetSlot
{
    DescriptorSetScope targetScope_ = DESCRIPTOR_SET_SCOPE_NONE;
    VKW::ProxySetHandle setHandle_;
};

template<typename TSlotOwner>
struct DescriptorSetSlotsOwner
{
    static constexpr DescriptorSetScope ownerScope_ = DescriptorSetSlotOwnerScope<TSlotOwner>();

    std::uint32_t slotsCount_;
    DescriptorSetSlot slots_[MaxScopeSets<TSlotOwner>()];
};

}
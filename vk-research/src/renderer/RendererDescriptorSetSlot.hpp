#pragma once


#include "RendererDescriptorSetScope.hpp"
#include "RendererDescriptorSet.hpp"



namespace Render
{

template<typename TSlotOwner>
struct DescriptorSetSlot
{
    constexpr DescriptorSetScope scope_ = DescriptorSetSlotOwnerScope<TSlotOwner>();
};

}
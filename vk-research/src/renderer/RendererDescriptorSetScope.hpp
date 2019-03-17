#pragma once

#include <cstdint>

namespace Render
{

enum DescriptorSetScope
{
    DESCRIPTOR_SET_SCOPE_ROOT,
    DESCRIPTOR_SET_SCOPE_RENDERPASS,
    DESCRIPTOR_SET_SCOPE_MATERIALTEMPLATE,
    DESCRIPTOR_SET_SCOPE_MATERIALINSTANCE,
    DESCRIPTOR_SET_SCOPE_RENDERITEM
};

static constexpr std::uint32_t SCOPE_MAX_SET_SLOTS_ROOT = 6;
static constexpr std::uint32_t SCOPE_MAX_SET_SLOTS_RENDERPASS = 6;
static constexpr std::uint32_t SCOPE_MAX_SET_SLOTS_MATERIALTEMPLATE = 6;
static constexpr std::uint32_t SCOPE_MAX_SET_SLOTS_MATERIALINSTANCE = 6;
static constexpr std::uint32_t SCOPE_MAX_SET_SLOTS_RENDERITEM = 6;



struct Material;
struct MaterialTemplate;
struct RenderItem;

class Root;
class Pass;

template<typename TSlotOwner>
constexpr DescriptorSetScope DescriptorSetSlotOwnerScope();

template<>
constexpr DescriptorSetScope DescriptorSetSlotOwnerScope<Root>()
{
    return DESCRIPTOR_SET_SCOPE_ROOT;
}

template<>
constexpr DescriptorSetScope DescriptorSetSlotOwnerScope<Pass>()
{
    return DESCRIPTOR_SET_SCOPE_RENDERPASS;
}

template<>
constexpr DescriptorSetScope DescriptorSetSlotOwnerScope<MaterialTemplate>()
{
    return DESCRIPTOR_SET_SCOPE_MATERIALTEMPLATE;
}

template<>
constexpr DescriptorSetScope DescriptorSetSlotOwnerScope<RenderItem>()
{
    return DESCRIPTOR_SET_SCOPE_RENDERITEM;
}

template<>
constexpr DescriptorSetScope DescriptorSetSlotOwnerScope<Material>()
{
    return DESCRIPTOR_SET_SCOPE_MATERIALINSTANCE;
}



template<typename TSlotOwner>
constexpr std::uint32_t MaxSetSlots();

template<>
constexpr std::uint32_t MaxSetSlots<Root>()
{
    return SCOPE_MAX_SET_SLOTS_ROOT;
}

template<>
constexpr std::uint32_t MaxSetSlots<Pass>()
{
    return SCOPE_MAX_SET_SLOTS_RENDERPASS;
}

template<>
constexpr std::uint32_t MaxSetSlots<MaterialTemplate>()
{
    return SCOPE_MAX_SET_SLOTS_MATERIALTEMPLATE;
}
template<>
constexpr std::uint32_t MaxSetSlots<Material>()
{
    return SCOPE_MAX_SET_SLOTS_MATERIALINSTANCE;
}
template<>
constexpr std::uint32_t MaxSetSlots<RenderItem>()
{
    return SCOPE_MAX_SET_SLOTS_RENDERITEM;
}
}
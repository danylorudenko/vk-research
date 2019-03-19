#pragma once

#include <cstdint>

namespace Render
{

enum DescriptorSetScope
{
    DESCRIPTOR_SET_SCOPE_NONE,
    DESCRIPTOR_SET_SCOPE_MATERIALTEMPLATE,
    DESCRIPTOR_SET_SCOPE_ROOT,
    DESCRIPTOR_SET_SCOPE_RENDERPASS,
    DESCRIPTOR_SET_SCOPE_MATERIALINSTANCE,
    DESCRIPTOR_SET_SCOPE_RENDERITEM
};

static constexpr std::uint32_t SCOPE_MAX_SETS_MATERIALTEMPLATE = 15;
static constexpr std::uint32_t SCOPE_MAX_SETS_ROOT = 6;
static constexpr std::uint32_t SCOPE_MAX_SETS_RENDERPASS = 6;
static constexpr std::uint32_t SCOPE_MAX_SETS_MATERIALINSTANCE = 6;
static constexpr std::uint32_t SCOPE_MAX_SETS_RENDERITEM = 6;



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
constexpr std::uint32_t MaxScopeSets();

template<>
constexpr std::uint32_t MaxScopeSets<Root>()
{
    return SCOPE_MAX_SETS_ROOT;
}

template<>
constexpr std::uint32_t MaxScopeSets<Pass>()
{
    return SCOPE_MAX_SETS_RENDERPASS;
}

template<>
constexpr std::uint32_t MaxScopeSets<MaterialTemplate>()
{
    return SCOPE_MAX_SETS_MATERIALTEMPLATE;
}
template<>
constexpr std::uint32_t MaxScopeSets<Material>()
{
    return SCOPE_MAX_SETS_MATERIALINSTANCE;
}
template<>
constexpr std::uint32_t MaxScopeSets<RenderItem>()
{
    return SCOPE_MAX_SETS_RENDERITEM;
}
}
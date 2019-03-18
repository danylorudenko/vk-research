#pragma once

#include "..\vk_interface\runtime\DescriptorSet.hpp"
#include "RootDef.hpp"

namespace Render
{


constexpr std::size_t SET_MEMBER_NAME_MAX_LENGTH = 64;

struct UniformBufferSetMember
{
    UniformBufferHandle uniformBufferHandle_;
    std::uint8_t* hostBuffer_;
    std::uint32_t hostBufferSize_;
};

struct Texture2DSetMember
{
};

struct StorageBufferSetMember
{
};

struct DescriptorSet
{
    Render::SetLayoutKey setLayoutKey_;
    
    VKW::ProxySetHandle proxyDescriptorSetHandle_;

    struct {
        char name_[SET_MEMBER_NAME_MAX_LENGTH];
        union {
            UniformBufferSetMember uniformBuffer_;
            Texture2DSetMember texture2D_;
            StorageBufferSetMember storageBuffer_;
        } data_;
    } setMembers_[VKW::DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];

};

}
#pragma once

#include <cstdint>
#include <vulkan\vulkan.h>
#include <limits>

namespace VAL
{

struct DescriptorSetLayoutMemberInfo
{
    VkDescriptorType type_;
    std::uint32_t binding_;
};

struct DescriptorSetLayout
{
    static std::uint32_t constexpr MAX_SET_LAYOUT_MEMBERS = 6;
    
    VkDescriptorSetLayout handle_ = VK_NULL_HANDLE;
    std::uint32_t membersCount_;
    DescriptorSetLayoutMemberInfo membersInfo_[MAX_SET_LAYOUT_MEMBERS];

};

struct DescriptorSetLayoutHandle
{
    DescriptorSetLayout* layout_ = nullptr;
};


struct PipelineLayout
{
    static std::uint32_t constexpr MAX_PIPELINE_LAYOUT_MEMBERS = 6;

    VkPipelineLayout handle_ = VK_NULL_HANDLE;
    std::uint32_t membersCount_;
    DescriptorSetLayoutHandle setLayoutMembers_[MAX_PIPELINE_LAYOUT_MEMBERS];

};

struct PipelineLayoutHandle
{
    PipelineLayout* layout_ = nullptr;
};

}
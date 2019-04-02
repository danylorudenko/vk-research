#pragma once

#include "..\vk_interface\pipeline\DescriptorLayout.hpp"

namespace Render
{

struct SetLayout
{
    VKW::DescriptorSetLayoutHandle vkwSetLayoutHandle_;

    std::uint32_t membersCount_;
    struct Member
    {
        VKW::DescriptorType type_;
        std::uint32_t binding_;
    }
    membersInfo_[VKW::DescriptorSetLayout::MAX_SET_LAYOUT_MEMBERS];
};

}
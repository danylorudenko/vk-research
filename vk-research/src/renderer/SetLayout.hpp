#pragma once

#include "..\class_features\NonCopyable.hpp"
#include "..\vk_interface\pipeline\DescriptorLayout.hpp"

namespace Render
{

class SetLayout
    : public NonCopyable
{
public:
    SetLayout();
    SetLayout(VKW::DescriptorSetLayoutHandle handle);
    SetLayout(SetLayout&& rhs);
    SetLayout& operator=(SetLayout&& rhs);

    VKW::DescriptorSetLayoutHandle vkSetLayout_;
};

}
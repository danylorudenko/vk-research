#pragma once

#include "..\vk_interface\pipeline\DescriptorLayout.hpp"

namespace Render
{

struct SetLayout
{
    VKW::DescriptorSetLayoutHandle vkwSetLayoutHandle_;
};

}
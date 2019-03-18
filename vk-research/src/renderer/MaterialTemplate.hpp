#pragma once

#include "RendererDescriptorSetSlotsOwner.hpp"

namespace Render
{

struct MaterialTemplate
{
    DescriptorSetSlotsOwner<MaterialTemplate> descriptorSetSlots_; 
};

}
#pragma once

#include "RendererDescriptorSetSlotsOwner.hpp"

namespace Render
{

struct Material
{
    DescriptorSetSlotsOwner<Material> descritorSetSlots_;
};

}

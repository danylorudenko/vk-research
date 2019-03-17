#pragma once

#include "RendererDescriptorSetSlot.hpp"

namespace Render
{

struct Material
{
    DescriptorSetSlot<Material> descriptorSetSlots[MaxSetSlots<Material>()];
};

}

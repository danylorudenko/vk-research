#pragma once

#include <renderer\MaterialTemplate.hpp>
#include <renderer\RendererDescriptorSetsOwner.hpp>
#include <renderer\RootDef.hpp>

namespace Render
{

struct Material
{
    MaterialTemplateKey templateKey_;

    std::uint32_t perPassDataCount_;
    struct PerPassData
    {
        DescriptorSetsOwner descritorSetsOwner_;
    }
    perPassData_[MATERIAL_TEMPLATE_PASS_LIMIT];
    
};

}

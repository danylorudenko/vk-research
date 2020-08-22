#pragma once

#include <renderer\SetLayout.hpp>
#include <renderer\RootDef.hpp>

namespace Render
{

constexpr std::uint32_t MATERIAL_TEMPLATE_PASS_LIMIT = 6;

struct MaterialTemplate
{
    std::uint32_t perPassDataCount_;
    struct PerPassData 
    {
        PassKey passKey_;
        PipelineKey pipelineKey_;

        std::uint32_t materialLayoutKeysCount_;
        SetLayoutKey materialLayoutKeys_[VKW::PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
        std::uint32_t renderitemLayoutKeysCount_;
        SetLayoutKey renderitemLayoutKeys_[VKW::PipelineLayout::MAX_PIPELINE_LAYOUT_MEMBERS];
    } perPassData_[MATERIAL_TEMPLATE_PASS_LIMIT];
};


}
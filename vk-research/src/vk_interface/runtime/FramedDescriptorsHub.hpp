#pragma once

#include <vector>
#include "../runtime/DescriptorSet.hpp"
#include "../resources/Framebuffer.hpp"

namespace VKW
{

struct FramedDescriptors
{
    std::vector<DescriptorSetHandle> descriptorSets_;
    std::vector<FramebufferHandle> framebuffers_;
};


struct FramedDescriptorsHub
{
    static std::uint32_t constexpr MAX_FRAMES_COUNT = 3;

    std::uint32_t framesCount_ = 0;
    FramedDescriptors contexts_[MAX_FRAMES_COUNT];
};

}
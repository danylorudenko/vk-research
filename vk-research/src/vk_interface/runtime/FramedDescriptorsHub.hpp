#pragma once

#include <vector>
#include "..\VkInterfaceConstants.hpp"
#include "..\buffer\BufferView.hpp"
#include "..\image\ImageView.hpp"
#include "..\runtime\DescriptorSet.hpp"
#include "..\resources\Framebuffer.hpp"

namespace VKW
{

struct FramedDescriptors
{
    std::vector<BufferViewHandle> bufferViews_;
    std::vector<ImageViewHandle> imageViews_;
    std::vector<DescriptorSetHandle> descriptorSets_;
    std::vector<FramebufferHandle> framebuffers_;
};


struct FramedDescriptorsHub
{
    static std::uint32_t constexpr MAX_FRAMES_COUNT = CONSTANTS::MAX_FRAMES_BUFFERING;

    std::uint32_t framesCount_ = 0;

    std::uint32_t bufferViewsNextId_ = 0;
    std::uint32_t imageViewsNextId_ = 0;
    std::uint32_t descriptorSetsNextId_ = 0;
    std::uint32_t framebuffersNextId_ = 0;

    FramedDescriptors contexts_[MAX_FRAMES_COUNT];
};

}
#pragma once

#include <vk_interface\VkInterfaceConstants.hpp>
#include <vk_interface\buffer\BufferView.hpp>
#include <vk_interface\image\ImageView.hpp>
#include <vk_interface\runtime\DescriptorSet.hpp>
#include <vk_interface\resources\Framebuffer.hpp>

#include <vector>

namespace VKW
{

struct FramedDescriptors
{
    std::vector<BufferViewHandle> bufferViews_;
    std::vector<ImageViewHandle> imageViews_;
    std::vector<DescriptorSetHandle> descriptorSets_;
    std::vector<Framebuffer*> framebuffers_;
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
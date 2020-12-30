#pragma once

#include <vector>
#include <utility>
#include <class_features\NonCopyable.hpp>

#include <vk_interface\image\ImageView.hpp>
#include <vk_interface\resources\ResourcesController.hpp>

namespace VKW
{

class ImportTable;
class Device;
class Swapchain;
class Surface;

struct SwapchainImageViewDesc
{
    std::uint32_t index_;
};

struct ImageViewDesc
{
    VkFormat format_;
    std::uint32_t width_;
    std::uint32_t height_;
    //std::uint32_t depth_;
    //std::uint32_t arrayLayers_;
    //std::uint32_t mipmaps_;
    ImageUsage usage_;
};



struct ImagesProviderDesc
{
    ImportTable* table_;
    Device* device_;
    Surface* surface_;
    Swapchain* swapchain_;
    ResourcesController* resourcesController_;
};

class ImagesProvider
{
public:
    ImagesProvider();
    ImagesProvider(ImagesProviderDesc const& desc);

    ImagesProvider(ImagesProvider&& rhs);
    ImagesProvider& operator=(ImagesProvider&& rhs);

    ImageView* RegisterSwapchainImageView(SwapchainImageViewDesc const& desc);
    std::vector<ImageView*> AcquireImageViews(std::uint32_t count, ImageViewDesc const* descs);
    void ReleaseImageViews(std::uint32_t count, ImageView** handles);

    VkSampler DefaultSamplerHandle() const;

    ~ImagesProvider();


private:
    struct ImageViewContainer
    {
        ImageView* view_;
        bool isSwapchain_;
    };

private:
    ImportTable* table_;
    Device* device_;
    Swapchain* swapchain_;
    ResourcesController* resourcesController_;

    VkSampler defaultSampler_;

    std::vector<ImageViewContainer> imageViewContainers_;
};

}
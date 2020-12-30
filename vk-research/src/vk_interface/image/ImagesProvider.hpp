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

    ImageView* RegisterSwapchainImageView(std::uint32_t index);
    std::vector<ImageView*> AllocateImagesAndViews(std::uint32_t count, VkFormat format, std::uint32_t width, std::uint32_t height, ImageUsage usage);
    void ReleaseImagesAndViews(std::uint32_t count, ImageView** handles);
    void ReleaseImagesAndViews(std::vector<ImageView*> handles);

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
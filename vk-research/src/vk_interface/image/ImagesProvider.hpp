#pragma once

#include <vector>
#include <utility>
#include "..\..\class_features\NonCopyable.hpp"

#include "ImageView.hpp"
#include "..\resources\ResourcesController.hpp"

namespace VKW
{

class ImportTable;
class Device;


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
    ResourcesController* resourcesController_;
};

class ImagesProvider
{
public:
    ImagesProvider();
    ImagesProvider(ImagesProviderDesc const& desc);

    ImagesProvider(ImagesProvider&& rhs);
    ImagesProvider& operator=(ImagesProvider&& rhs);

    ImageViewHandle AcquireImage(ImageViewDesc const& desc);
    void ReleaseImage(ImageViewHandle handle);

    ImageView* GetImageView(ImageViewHandle handle);

    VkSampler DefaultSamplerHandle() const;

    ~ImagesProvider();


private:
    struct ImageViewContainer
    {
        ImageView* view_;
        ImageResourceHandle resource_;
    };

private:
    ImportTable* table_;
    Device* device_;
    ResourcesController* resourcesController_;

    VkSampler defaultSampler_;

    std::vector<ImageViewContainer> imageViewContainers_;
};

}
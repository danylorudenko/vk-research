#pragma once

#include <class_features\NonCopyable.hpp>
#include <vk_interface\resources\Resource.hpp>

#include <vulkan\vulkan.h>
#include <unordered_set>
#include <unordered_map>

namespace VKW
{

enum class BufferUsage
{
    VERTEX_INDEX,
    VERTEX_INDEX_WRITABLE,
    UNIFORM,
    UPLOAD_BUFFER
};

struct BufferDesc
{
    std::uint32_t size_;
    BufferUsage usage_;
};

enum class ImageUsage
{
    TEXTURE,
    STORAGE_IMAGE,
    STORAGE_IMAGE_READONLY,
    RENDER_TARGET,
    DEPTH,
    STENCIL,
    DEPTH_STENCIL,
    UPLOAD_IMAGE
};

struct ImageDesc
{
    std::uint32_t width_;
    std::uint32_t height_;
    VkFormat format_;
    ImageUsage usage_;
};

class ImportTable;
class Device;
class MemoryController;

struct ResourcesControllerDesc
{
    ImportTable* table_;
    Device* device_;
    MemoryController* memoryController_;
};

class ResourcesController
    : public NonCopyable
{
public:
    ResourcesController();
    ResourcesController(ResourcesControllerDesc const& desc);

    ResourcesController(ResourcesController&& rhs);
    ResourcesController& operator=(ResourcesController&& rhs);

    BufferResource* CreateBuffer(BufferDesc const& desc);
    void FreeBuffer(BufferResource* handle);

    ImageResource* CreateImage(ImageDesc const& desc);
    void FreeImage(ImageResource* handle);

    ImageResourceView* ViewImageAs(
        ImageResource* resource, 
        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, 
        VkImageSubresourceRange const* subresource = nullptr, 
        VkFormat const* format = nullptr, 
        VkImageViewType const* type = nullptr, 
        VkComponentMapping const* mapping = nullptr
    );
    void FreeImageView(ImageResourceView* view);

    ~ResourcesController();

public:
    VkImageViewType         ImageTypeToViewType(VkImageType type, std::uint32_t arrayLayers);
    VkComponentMapping      DefaultComponentMapping();
    VkImageSubresourceRange DefaultSubresourceRange(ImageResource const* resource, VkImageAspectFlags aspectFlags);

private:
    ImportTable* table_;
    Device* device_;

    MemoryController* memoryController_;

    std::unordered_set<BufferResource*> buffers_;
    std::unordered_set<ImageResource*> images_;
    std::unordered_multimap<ImageResource*, ImageResourceView*> imageViewMap_;


};

VkImageViewType ImageTypeToViewType(VkImageType type, std::uint32_t arrayLayers);
VkComponentMapping DefaultComponentMapping();
VkImageSubresourceRange DefaultSubresourceRange(ImageResource const* resource, VkImageAspectFlags aspectFlags);

}
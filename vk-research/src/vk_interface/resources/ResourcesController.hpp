#pragma once

#include "..\..\class_features\NonCopyable.hpp"
#include "Resource.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace VKW
{

enum class BufferUsage
{
    VERTEX_INDEX,
    UNIFORM,
    UPLOAD_BUFFER
};

struct BufferDesc
{
    std::uint64_t size_;
    BufferUsage usage_;
};

enum class ImageUsage
{
    TEXTURE,
    RENDER_TARGET,
    DEPTH_STENCIL,
    SWAP_CHAIN_RENDER_TARGET
};

struct ImageDesc
{
    std::uint64_t width_;
    std::uint64_t height_;
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

    BufferResourceHandle CreateBuffer(BufferDesc const& desc);
    void FreeBuffer(BufferResourceHandle handle);

    ImageResourceHandle CreateImage(ImageDesc const& desc);
    void FreeImage(ImageResourceHandle handle);

    BufferResource* GetBuffer(BufferResourceHandle handle);
    ImageResource* GetImage(ImageResourceHandle handle);

    ~ResourcesController();

private:
    ImportTable* table_;
    Device* device_;

    MemoryController* memoryController_;

    std::vector<BufferResource*> buffers_;
    std::vector<ImageResource*> images_;


};


}
#pragma once

#include "..\..\class_features\NonCopyable.hpp"
#include "Resource.hpp"

#include <vulkan\vulkan.h>
#include <vector>

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
    RENDER_TARGET,
    DEPTH,
    STENCIL,
    DEPTH_STENCIL
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
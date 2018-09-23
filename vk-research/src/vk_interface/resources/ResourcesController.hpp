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
    TEXTURE
};

struct ImageDesc
{
    std::uint64_t width_;
    std::uint64_t height_;
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
    ImageResourceHandle CreateImage(ImageDesc const& desc);

    BufferResource* GetBuffer(BufferResourceHandle handle) const;
    ImageResource* GetImage(ImageResourceHandle handle) const;

    ~ResourcesController();

private:
    ImportTable* table_;
    Device* device_;

    MemoryController* memoryController_;

    std::vector<BufferResource> staticBuffers_;
    std::vector<ImageResource> staticImages_;


};


}
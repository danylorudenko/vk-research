#pragma once

#include "../../class_features/NonCopyable.hpp"
#include "BufferView.hpp"
#include "../resources/Resource.hpp"

#include <vector>

namespace VKW
{

class ImportTable;
class Device;
class ResourcesController;

struct BufferViewDesc
{
    VkFormat format_;
    std::uint64_t offset_;
    std::uint64_t size_;
    BufferUsage usage_;
};


struct BuffersProviderDesc
{
    ImportTable* table_;
    Device* device_;
    ResourcesController* resourcesController_;
};

class BuffersProvider
{
public:
    BuffersProvider();
    BuffersProvider(BuffersProviderDesc const& desc);

    BuffersProvider(BuffersProvider&& rhs);
    BuffersProvider& operator=(BuffersProvider&& rhs);
    
    // Should not recieve BufferResourceHandle as a parameter: it is provider's responisbility to create resources if needed.
    void AcquireBuffers(std::uint32_t buffersCount, BufferViewDesc const* desc, BufferViewHandle* results);
    void ReleaseBuffers(std::uint32_t buffersCount, BufferViewHandle const* handles);

    void RegisterViews(std::uint32_t buffersCount, BufferResourceHandle* buffers, BufferViewDesc const* desc, BufferViewHandle* results);

    ~BuffersProvider();

private:
    ImportTable* table_;
    Device* device_;
    ResourcesController* resourcesController_;

    std::vector<BufferView> bufferViews_;
};

}
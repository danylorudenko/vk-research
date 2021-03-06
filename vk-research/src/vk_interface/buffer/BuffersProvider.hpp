#pragma once

#include "..\..\class_features\NonCopyable.hpp"
#include "BufferView.hpp"
#include "..\resources\ResourcesController.hpp"

#include <vector>

namespace VKW
{

class ImportTable;
class Device;


struct BufferViewDesc
{
    VkFormat format_;
    std::uint32_t size_;
    BufferUsage usage_;
};




struct BuffersProviderDesc
{
    ImportTable* table_;
    Device* device_;
    ResourcesController* resourcesController_;
};

class BuffersProvider
    : public NonCopyable
{

public:
    BuffersProvider();
    BuffersProvider(BuffersProviderDesc const& desc);

    BuffersProvider(BuffersProvider&& rhs);
    BuffersProvider& operator=(BuffersProvider&& rhs);
    
    void AcquireViews(std::uint32_t buffersCount, BufferViewDesc const* desc, BufferViewHandle* results);
    void ReleaseViews(std::uint32_t buffersCount, BufferViewHandle const* handles);

    BufferView* GetView(BufferViewHandle handle);
    BufferResource* GetViewResource(BufferViewHandle handle);

    ~BuffersProvider();


private:
    ImportTable* table_;
    Device* device_;
    ResourcesController* resourcesController_;

    std::vector<BufferView*> bufferViews_;
    std::vector<ProvidedBuffer*> providedBuffers_;
};

}
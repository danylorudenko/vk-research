#pragma once

#include <unordered_set>

#include <class_features\NonCopyable.hpp>

#include <vk_interface\buffer\BufferView.hpp>
#include <vk_interface\resources\ResourcesController.hpp>

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
    
    void CreateViewsAndCreateBuffers(std::uint32_t buffersCount, BufferViewDesc const* desc, BufferView** results);
    void ReleaseViewsAndBuffers(std::uint32_t buffersCount, BufferView** views);

    ~BuffersProvider();


private:
    ImportTable* table_;
    Device* device_;
    ResourcesController* resourcesController_;

    std::unordered_set<BufferView*> bufferViews_;
};

}
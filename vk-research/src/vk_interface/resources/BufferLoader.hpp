#pragma once

#include "..\..\class_features\NonCopyable.hpp"
#include "Buffer.hpp"
#include "..\VulkanImportTable.hpp"

namespace VKW
{

class Device;

class BufferLoader
    : public NonCopyable
{
public:
    BufferLoader();
    BufferLoader(VulkanImportTable* table, Device* device);

    BufferLoader(BufferLoader&& rhs);
    BufferLoader& operator=(BufferLoader&& rhs);

    Buffer LoadBuffer(BufferCreateInfo const& desc);
    void UnloadBuffer(Buffer& buffer);

    ~BufferLoader();

private:
    VulkanImportTable* table_;
    Device* device_;
};

}
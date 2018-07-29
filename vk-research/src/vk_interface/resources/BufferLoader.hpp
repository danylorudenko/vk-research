#pragma once

#include "..\..\class_features\NonCopyable.hpp"
#include "..\ImportTable.hpp"
#include "..\memory\MemoryController.hpp"

namespace VKW
{



enum class BufferUsage
{
    VERTEX_INDEX,
    UNIFORM,
    UPLOAD_BUFFER
};

struct Buffer
{
    VkBuffer handle_;
};

struct BufferCreateInfo
{
    std::uint64_t size_;
    BufferUsage usage_;
};



class Device;

struct BufferLoaderDesc
{
    ImportTable* table_;
    Device* device_;
    MemoryController* memoryController_;
};

class BufferLoader
    : public NonCopyable
{
public:
    BufferLoader();
    BufferLoader(BufferLoaderDesc const& desc);

    BufferLoader(BufferLoader&& rhs);
    BufferLoader& operator=(BufferLoader&& rhs);

    Buffer LoadBuffer(BufferCreateInfo const& desc);
    void UnloadBuffer(Buffer& buffer);

    ~BufferLoader();

private:
    ImportTable* table_;
    Device* device_;
    MemoryController* memoryController_;
};

}
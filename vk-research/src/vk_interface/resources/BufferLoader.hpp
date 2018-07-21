#pragma once

#include "..\..\class_features\NonCopyable.hpp"
#include "..\ImportTable.hpp"
#include "Buffer.hpp"
#include "..\memory\MemoryController.hpp"

namespace VKW
{

class Device;

class BufferLoader
    : public NonCopyable
{
public:
    BufferLoader();
    BufferLoader(ImportTable* table, Device* device, MemoryController* memoryController);

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
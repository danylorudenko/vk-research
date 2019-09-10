#pragma once

#include "..\..\class_features\NonCopyable.hpp"
#include "Resource.hpp"

#include <cstdint>
#include <vector>

namespace VAL
{


class ImportTable;
class Device;
class MemoryController;


struct DynamicResourceStorageDesc
{
    ImportTable* table_;
    Device* device_;
    MemoryController* memoryController_;
    std::uint64_t defaultSize_;
};

class DynamicResourceStorage
    : public NonCopyable
{
public:
    struct StorageHandle
    {
        std::uint32_t id_;
    };

    struct Storage
    {
        Storage(VkBuffer buffer, std::uint32_t size, std::uint32_t freeOffset, std::uint16_t subresourceCount);

        VkBuffer buffer_;
        std::uint32_t size_;
        std::uint32_t freeOffset_;
        std::uint16_t subresourcesCount_;
    };


public:
    DynamicResourceStorage();
    DynamicResourceStorage(DynamicResourceStorageDesc const& desc);

    DynamicResourceStorage(DynamicResourceStorage&& rhs);
    DynamicResourceStorage& operator=(DynamicResourceStorage&& rhs);

    ~DynamicResourceStorage();

    void AllocSubresourcePatch(std::uint32_t count, std::uint32_t size, SubbufferResource* output);
    void FreeSubresourcePatch(std::uint32_t count, SubbufferResource* subresources);


private:
    StorageHandle AllocateStorage(std::uint32_t size);
    void FreeStorage(StorageHandle index);

private:
    ImportTable* table_;
    Device* device_;
    MemoryController* memoryController_;

    std::uint64_t defaultContainerSize_;

    std::vector<Storage> storages_;
};

}
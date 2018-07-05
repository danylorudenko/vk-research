#pragma once

#include "../Device.hpp"

namespace VKW
{

class MemoryController
    : public NonCopyable
{
public:
    MemoryController();
    MemoryController(VulkanImportTable* table, Device* device);

    MemoryController(MemoryController&& rhs);
    MemoryController& operator=(MemoryController&& rhs);

    ~MemoryController();

private:
    VulkanImportTable* table_;
    Device* device_;
};

}
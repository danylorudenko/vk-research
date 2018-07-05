#include "MemoryController.hpp"

namespace VKW
{

MemoryController::MemoryController()
    : table_{ nullptr }
    , device_{ nullptr }
{ 
}

MemoryController::MemoryController(VulkanImportTable* table, Device* device)
    : table_{ table }
    , device_{ device }
{

}

MemoryController::MemoryController(MemoryController&& rhs)
{
    operator=(std::move(rhs));
}

MemoryController& MemoryController::operator=(MemoryController&& rhs)
{
    std::swap(table_, rhs.table_);
    std::swap(device_, rhs.device_);

    return *this;
}

MemoryController::~MemoryController()
{
}

}
#include "ContinuousDataStorage.hpp"

HandleTable::HandleTable(std::uint32_t reserve)
{
    table_.reserve(reserve);
}

HandleTable::HandleTable(HandleTable const&) = default;
HandleTable::HandleTable(HandleTable&&) = default;
HandleTable& HandleTable::operator=(HandleTable const&) = default;
HandleTable& HandleTable::operator=(HandleTable&&) = default;
HandleTable::~HandleTable() = default;

void HandleTable::RegisterStorageHandle(std::uint32_t id)
{

}

void HandleTable::ReleaseStorageHandle(StorageHandle handle)
{

}
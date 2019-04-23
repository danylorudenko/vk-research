#pragma once

#include <utility>
#include <cstdint>
#include <limits>
#include <cstdlib>
#include <vector>

template<typename T>
struct Handle
{
    std::uint32_t id_ = 0;
};

struct StorageHandle
{
    std::uint32_t id_ = std::numeric_limits<std::uint32_t>::max();
};

class HandleTable
{
public:
    HandleTable(std::uint32_t reserve);

    HandleTable(HandleTable const& rhs);
    HandleTable(HandleTable&& rhs);
    HandleTable& operator=(HandleTable const& rhs);
    HandleTable& operator=(HandleTable&& rhs);
    ~HandleTable();

    void RegisterStorageHandle(std::uint32_t id);
    void ReleaseStorageHandle(StorageHandle handle);

private:
    std::vector<StorageHandle> table_;
};

template<typename T>
class ContinuousDataStorage
{
    static_assert(sizeof(T) >= sizeof(std::uint32_t), "ContinuosDataStorage does not support types less than 4 bytes in size.");

    using InternalHandle = std::uint32_t;

public:
    ContinuousDataStorage(std::uint32_t dataCount)
        : storage_{ nullptr }
        , freeDataHandle_{ 0 }
        , maxObjects_{ dataCount }
    {
        if (dataCount == 0)
            return;
        
        storage_ = std::malloc(sizeof(T) * dataCount);

        // creating linked list in the free storage
        auto const iterationsCount = dataCount - 1;
        for (auto i = 0u; i < iterationsCount; ++i) {
            auto* dataLocal = reinterpret_cast<InternalHandle*>(storage_ + i);
            *dataLocal = i + 1;
        }
        *(reinterpret_cast<InternalHandle*>(storage_ + iterationsCount)) = 0;
    }

    ContinuousDataStorage(ContinuousDataStorage&& rhs)
    {
        operator=(std::move(rhs));
    }

    ContinuousDataStorage& operator=(ContinuousDataStorage&& rhs)
    {
        std::swap(storage_, rhs.storage_);
        std::swap(freeDataHandle_, rhs.freeDataHandle_);
        std::swap(maxObjects_, rhs.maxObjects_);

        return *this;
    }

    ~ContinuousDataStorage()
    {
        std::free(sizeof(T) * maxObjects_);
    }

    T* AccessData(Handle<T> handle)
    {
        return storage_ + handle.id_;
    }

    Handle<T> AcquireData()
    {
        InternalHandle const acquiredHandle = freeDataHandle_.id_;

        auto* acquiredData = storage_ + acquiredHandle;
        freeDataHandle_ = *(reinterpret_cast<InternalHandle*>(acquiredData));

        return Handle<T>{ acquiredHandle };
    }

    void ReleaseData(Handle<T> releasedHandle)
    {
        std::uint32_t* releasedData = reinterpret_cast<InternalHandle*>(storage_ + releasedHandle.id_);
        *releasedData = freeDataHandle_;
        freeDataHandle_ = releasedHandle.id_;
    }


private:
    T* storage_;
    InternalHandle freeDataHandle_;
    std::uint32_t maxObjects_;
};
#pragma once

#include <memory>

#include "..\class_features\NonCopyable.hpp"
#include "..\system\DynamicLibrary.hpp"

#include "ImportTable.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "memory/MemoryController.hpp"
#include "resources/BufferLoader.hpp"

namespace VKW
{

class Loader final
    : public NonCopyable
{
public:
    Loader(bool debug = false);

    Loader(Loader&& rhs) = default;
    Loader& operator=(Loader&& rhs) = default;

    ~Loader();

    VKW::ImportTable const& Table() const;
    VKW::Device& Device();

    VKW::MemoryController& MemoryController();
    VKW::BufferLoader& BufferLoader();
    

private:
    std::unique_ptr<DynamicLibrary> vulkanLibrary_;
    std::unique_ptr<VKW::ImportTable> table_;

    std::unique_ptr<VKW::Instance> instance_;
    std::unique_ptr<VKW::Device> device_;

    std::unique_ptr<VKW::MemoryController> memoryController_;
    std::unique_ptr<VKW::BufferLoader> bufferLoader_;

};

}
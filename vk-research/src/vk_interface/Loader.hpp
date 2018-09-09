#pragma once

#include <memory>

#include "..\class_features\NonCopyable.hpp"
#include "..\system\DynamicLibrary.hpp"

#include "ImportTable.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "memory/MemoryController.hpp"
#include "resources/BufferLoader.hpp"
#include "worker/WorkersProvider.hpp"

class IOManager;

namespace VKW
{

struct LoaderDesc
{
    IOManager* ioManager_;
    bool debug_ = false;
};

class Loader final
    : public NonCopyable
{
public:
    Loader(LoaderDesc const& desc);

    Loader(Loader&& rhs) = default;
    Loader& operator=(Loader&& rhs) = default;

    ~Loader();

    VKW::ImportTable const& Table() const;
    VKW::Device& Device();

    VKW::MemoryController& MemoryController();
    VKW::BufferLoader& BufferLoader();
    VKW::WorkersProvider& WorkersProvider();
    

private:
    std::unique_ptr<DynamicLibrary> vulkanLibrary_;
    std::unique_ptr<VKW::ImportTable> table_;

    std::unique_ptr<VKW::Instance> instance_;
    std::unique_ptr<VKW::Device> device_;

    std::unique_ptr<VKW::MemoryController> memoryController_;
    std::unique_ptr<VKW::BufferLoader> bufferLoader_;

    std::unique_ptr<VKW::WorkersProvider> workersProvider_;
};

}
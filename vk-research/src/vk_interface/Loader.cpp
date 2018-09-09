#include "Loader.hpp"
#include <iostream>


namespace VKW
{

Loader::Loader(LoaderDesc const& desc)
    : vulkanLibrary_{ std::make_unique<DynamicLibrary>("vulkan-1.dll") }
    , table_{ std::make_unique<ImportTable>(*vulkanLibrary_) }
{

    auto instanceExtensions = std::vector<std::string>{ "VK_KHR_surface", VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
    if (desc.debug_)
        instanceExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    auto instanceLayers = std::vector<std::string>{};
    if (desc.debug_) {
        instanceLayers.emplace_back("VK_LAYER_LUNARG_standard_validation");
    }



    VKW::InstanceDesc instanceDesc;
    instanceDesc.table_ = table_.get();
    instanceDesc.requiredInstanceExtensions_ = instanceExtensions;
    instanceDesc.requiredInstanceLayers_ = instanceLayers;
    instanceDesc.debug_ = desc.debug_;

    instance_ = std::make_unique<VKW::Instance>(instanceDesc);



    VKW::DeviceDesc deviceDesc;
    deviceDesc.table_ = table_.get();
    deviceDesc.instance_ = instance_.get();
    deviceDesc.requiredExtensions_ = { "VK_KHR_swapchain" };
    deviceDesc.graphicsQueueCount_ = 1;
    deviceDesc.computeQueueCount_ = 0;
    deviceDesc.transferQueueCount_ = 0;

    device_ = std::make_unique<VKW::Device>(deviceDesc);
    


    VKW::MemoryControllerDesc memoryControllerDesc;
    memoryControllerDesc.table_ = table_.get();
    memoryControllerDesc.device_ = device_.get();

    memoryController_ = std::make_unique<VKW::MemoryController>(memoryControllerDesc);



    VKW::BufferLoaderDesc bufferLoaderDesc;
    bufferLoaderDesc.table_ = table_.get();
    bufferLoaderDesc.device_ = device_.get();
    bufferLoaderDesc.memoryController_ = memoryController_.get();

    bufferLoader_ = std::make_unique<VKW::BufferLoader>(bufferLoaderDesc);



    VKW::WorkersProviderDesc wcsDesc;
    wcsDesc.table_ = table_.get();
    wcsDesc.device_ = device_.get();
    wcsDesc.graphicsQueueCount_ = 1;
    wcsDesc.computeQueueCount_ = 0;
    wcsDesc.transferQueueCount_ = 0;

    workersProvider_ = std::make_unique<VKW::WorkersProvider>(wcsDesc);
}

Loader::~Loader()
{
    
}

ImportTable const& Loader::Table() const
{
    return *table_;
}

Device& Loader::Device()
{
    return *device_;
}


VKW::MemoryController& Loader::MemoryController()
{
    return *memoryController_;
}

VKW::BufferLoader& Loader::BufferLoader()
{
    return *bufferLoader_;
}

VKW::WorkersProvider& Loader::WorkersProvider()
{
    return *workersProvider_;
}


}
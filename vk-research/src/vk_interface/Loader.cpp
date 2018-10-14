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
    deviceDesc.graphicsPresentSupportRequired_ = true;
    deviceDesc.graphicsQueueCount_ = 1;
    deviceDesc.computeQueueCount_ = 0;
    deviceDesc.transferQueueCount_ = 0;

    device_ = std::make_unique<VKW::Device>(deviceDesc);
    


    VKW::SurfaceDesc surfaceDesc;
    surfaceDesc.table_ = table_.get();
    surfaceDesc.instance_ = instance_.get();
    surfaceDesc.device_ = device_.get();
    surfaceDesc.hInstance_ = desc.hInstance_;
    surfaceDesc.hwnd_ = desc.hwnd_;

    surface_ = std::make_unique<VKW::Surface>(surfaceDesc);



    VKW::SwapchainDesc swapchainDesc;
    swapchainDesc.table_ = table_.get();
    swapchainDesc.device_ = device_.get();
    swapchainDesc.surface_ = surface_.get();
    swapchainDesc.imagesCount_ = desc.bufferingCount_;

    swapchain_ = std::make_unique<VKW::Swapchain>(swapchainDesc);



    VKW::WorkersProviderDesc wcsDesc;
    wcsDesc.table_ = table_.get();
    wcsDesc.device_ = device_.get();
    wcsDesc.graphicsQueueCount_ = 1;
    wcsDesc.computeQueueCount_ = 0;
    wcsDesc.transferQueueCount_ = 0;

    workersProvider_ = std::make_unique<VKW::WorkersProvider>(wcsDesc);



    VKW::MemoryControllerDesc memoryControllerDesc;
    memoryControllerDesc.table_ = table_.get();
    memoryControllerDesc.device_ = device_.get();

    memoryController_ = std::make_unique<VKW::MemoryController>(memoryControllerDesc);



    VKW::ResourcesControllerDesc resourcesControllerDesc;
    resourcesControllerDesc.table_ = table_.get();
    resourcesControllerDesc.device_ = device_.get();
    resourcesControllerDesc.memoryController_ = memoryController_.get();

    resourcesController_ = std::make_unique<VKW::ResourcesController>(resourcesControllerDesc);



    VKW::ShaderModuleFactoryDesc shaderModuleFactoryDesc;
    shaderModuleFactoryDesc.table_ = table_.get();
    shaderModuleFactoryDesc.device_ = device_.get();
    shaderModuleFactoryDesc.ioManager_ = desc.ioManager_;

    //shaderModuleFactory_ = std::make_unique<VKW::ShaderModuleFactory>(shaderModuleFactoryDesc);



    VKW::PipelineFactoryDesc pipelineFactoryDesc;
    pipelineFactoryDesc.table_ = table_.get();
    pipelineFactoryDesc.device_ = device_.get();

    //pipelineFactory_ = std::make_unique<VKW::PipelineFactory>(pipelineFactoryDesc);



    VKW::ResourceBindingServiceDesc resourceBindingServiceDesc;
    resourceBindingServiceDesc.table_ = table_.get();
    resourceBindingServiceDesc.device_ = device_.get();
    resourceBindingServiceDesc.framesCount_ = desc.bufferingCount_;

    //resourceBindingService_ = std::make_unique<VKW::ResourceBindingService>(resourceBindingServiceDesc);
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

VKW::WorkersProvider& Loader::WorkersProvider()
{
    return *workersProvider_;
}


}
#include "Loader.hpp"
#include <iostream>
#include "VkInterfaceConstants.hpp"
#include <Windows.h>


namespace VAL
{

Loader::Loader(LoaderDesc const& desc)
    : vulkanLibrary_{}
    , table_{}
{
    vulkanLibrary_ = std::make_unique<DynamicLibrary>("vulkan-1.dll");
    if (!vulkanLibrary_) {
        MessageBoxA(desc.hwnd_, "Failed to load vulkan-1.dll", NULL, MB_ICONERROR | MB_OK);
        return;
    }

    table_ = std::make_unique<ImportTable>(*vulkanLibrary_);

    auto instanceExtensions = std::vector<std::string>{ "VK_KHR_surface", VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
    if (desc.debug_)
        instanceExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    auto instanceLayers = std::vector<std::string>{};
    if (desc.debug_) {
        instanceLayers.emplace_back("VK_LAYER_LUNARG_standard_validation");
    }



    VAL::InstanceDesc instanceDesc;
    instanceDesc.table_ = table_.get();
    instanceDesc.requiredInstanceExtensions_ = instanceExtensions;
    instanceDesc.requiredInstanceLayers_ = instanceLayers;
    instanceDesc.debug_ = desc.debug_;

    instance_ = std::make_unique<VAL::Instance>(instanceDesc);



    VAL::DeviceDesc deviceDesc;
    deviceDesc.table_ = table_.get();
    deviceDesc.instance_ = instance_.get();
    deviceDesc.requiredExtensions_ = { "VK_KHR_swapchain" };
    deviceDesc.graphicsPresentQueueCount_ = 1;
    deviceDesc.computeQueueCount_ = 0;
    deviceDesc.transferQueueCount_ = 0;

    device_ = std::make_unique<VAL::Device>(deviceDesc);
    


    VAL::SurfaceDesc surfaceDesc;
    surfaceDesc.table_ = table_.get();
    surfaceDesc.instance_ = instance_.get();
    surfaceDesc.device_ = device_.get();
    surfaceDesc.hInstance_ = desc.hInstance_;
    surfaceDesc.hwnd_ = desc.hwnd_;

    surface_ = std::make_unique<VAL::Surface>(surfaceDesc);



    VAL::SwapchainDesc swapchainDesc;
    swapchainDesc.table_ = table_.get();
    swapchainDesc.device_ = device_.get();
    swapchainDesc.surface_ = surface_.get();
    swapchainDesc.imagesCount_ = desc.bufferingCount_;

    swapchain_ = std::make_unique<VAL::Swapchain>(swapchainDesc);




    VAL::MemoryControllerDesc memoryControllerDesc;
    memoryControllerDesc.table_ = table_.get();
    memoryControllerDesc.device_ = device_.get();

    memoryController_ = std::make_unique<VAL::MemoryController>(memoryControllerDesc);



    VAL::ResourcesControllerDesc resourcesControllerDesc;
    resourcesControllerDesc.table_ = table_.get();
    resourcesControllerDesc.device_ = device_.get();
    resourcesControllerDesc.memoryController_ = memoryController_.get();

    resourcesController_ = std::make_unique<VAL::ResourcesController>(resourcesControllerDesc);



    VAL::BuffersProviderDesc buffersProviderDesc;
    buffersProviderDesc.table_ = table_.get();
    buffersProviderDesc.device_ = device_.get();
    buffersProviderDesc.resourcesController_ = resourcesController_.get();

    buffersProvider_ = std::make_unique<VAL::BuffersProvider>(buffersProviderDesc);



    VAL::ImagesProviderDesc imagesProviderDesc;
    imagesProviderDesc.table_ = table_.get();
    imagesProviderDesc.device_ = device_.get();
    imagesProviderDesc.swapchain_ = swapchain_.get();
    imagesProviderDesc.resourcesController_ = resourcesController_.get();

    imagesProvider_ = std::make_unique<VAL::ImagesProvider>(imagesProviderDesc);



    framedDescriptorsHub_ = std::make_unique<VAL::FramedDescriptorsHub>();
    assert(swapchain_->ImageCount() <= CONSTANTS::MAX_FRAMES_BUFFERING);
    framedDescriptorsHub_->framesCount_ = swapchain_->ImageCount();



    VAL::RenderPassControllerDesc renderPassControllerDesc;
    renderPassControllerDesc.table_ = table_.get();
    renderPassControllerDesc.device_ = device_.get();

    renderPassController_ = std::make_unique<VAL::RenderPassController>(renderPassControllerDesc);



    VAL::FramebufferControllerDesc framebufferControllerDesc;
    framebufferControllerDesc.table_ = table_.get();
    framebufferControllerDesc.device_ = device_.get();
    framebufferControllerDesc.swapchain_ = swapchain_.get();
    framebufferControllerDesc.imagesProvider_ = imagesProvider_.get();
    framebufferControllerDesc.renderPassController_ = renderPassController_.get();

    framebufferController_ = std::make_unique<VAL::FramebufferController>(framebufferControllerDesc);



    VAL::DescriptorLayoutControllerDesc descriptorLayoutControllerDesc;
    descriptorLayoutControllerDesc.device_ = device_.get();
    descriptorLayoutControllerDesc.table_ = table_.get();

    descriptorLayoutController_ = std::make_unique<VAL::DescriptorLayoutController>(descriptorLayoutControllerDesc);



    VAL::DescriptorSetControllerDesc descriptorSetControllerDesc;
    descriptorSetControllerDesc.table_ = table_.get();
    descriptorSetControllerDesc.device_ = device_.get();
    descriptorSetControllerDesc.buffersProvider_ = buffersProvider_.get();
    descriptorSetControllerDesc.imagesProvider_ = imagesProvider_.get();
    descriptorSetControllerDesc.layoutController_ = descriptorLayoutController_.get();

    descriptorSetController_ = std::make_unique<VAL::DescriptorSetController>(descriptorSetControllerDesc);



    VAL::ResourceRendererProxyDesc resourceRendererProxyDesc;
    resourceRendererProxyDesc.table_ = table_.get();
    resourceRendererProxyDesc.device_ = device_.get();
    resourceRendererProxyDesc.memoryController_ = memoryController_.get();
    resourceRendererProxyDesc.resourcesController_ = resourcesController_.get();
    resourceRendererProxyDesc.buffersProvider_ = buffersProvider_.get();
    resourceRendererProxyDesc.imagesProvider_ = imagesProvider_.get();
    resourceRendererProxyDesc.layoutController_ = descriptorLayoutController_.get();
    resourceRendererProxyDesc.descriptorSetsController_ = descriptorSetController_.get();
    resourceRendererProxyDesc.renderPassController_ = renderPassController_.get();
    resourceRendererProxyDesc.framebufferController_ = framebufferController_.get();
    resourceRendererProxyDesc.framedDescriptorsHub_ = framedDescriptorsHub_.get();

    resourceRendererProxy_ = std::make_unique<VAL::ResourceRendererProxy>(resourceRendererProxyDesc);



    VAL::ShaderModuleFactoryDesc shaderModuleFactoryDesc;
    shaderModuleFactoryDesc.table_ = table_.get();
    shaderModuleFactoryDesc.device_ = device_.get();
    shaderModuleFactoryDesc.ioManager_ = desc.ioManager_;

    shaderModuleFactory_ = std::make_unique<VAL::ShaderModuleFactory>(shaderModuleFactoryDesc);



    VAL::PipelineFactoryDesc pipelineFactoryDesc;
    pipelineFactoryDesc.table_ = table_.get();
    pipelineFactoryDesc.device_ = device_.get();
    pipelineFactoryDesc.descriptorLayoutController_ = descriptorLayoutController_.get();
    pipelineFactoryDesc.renderPassController_ = renderPassController_.get();
    pipelineFactoryDesc.shaderModuleFactory_ = shaderModuleFactory_.get();

    pipelineFactory_ = std::make_unique<VAL::PipelineFactory>(pipelineFactoryDesc);



    VAL::WorkersProviderDesc wcsDesc;
    wcsDesc.table_ = table_.get();
    wcsDesc.device_ = device_.get();
    wcsDesc.bufferingCount_ = swapchain_->ImageCount();
    wcsDesc.graphicsPresentQueueCount_ = 1;
    wcsDesc.computeQueueCount_ = 0;
    wcsDesc.transferQueueCount_ = 0;

    workersProvider_ = std::make_unique<VAL::WorkersProvider>(wcsDesc);



    VAL::PresentationControllerDesc presentationControllerDesc;
    presentationControllerDesc.table_ = table_.get();
    presentationControllerDesc.device_ = device_.get();
    presentationControllerDesc.swapchain_ = swapchain_.get();
    presentationControllerDesc.presentationWorker_ = workersProvider_->PresentWorker();

    presentationController_ = std::make_unique<VAL::PresentationController>(presentationControllerDesc);



    VAL::ResourceBindingServiceDesc resourceBindingServiceDesc;
    resourceBindingServiceDesc.table_ = table_.get();
    resourceBindingServiceDesc.device_ = device_.get();
    resourceBindingServiceDesc.framesCount_ = swapchain_->ImageCount();

    //resourceBindingService_ = std::make_unique<VAL::ResourceBindingService>(resourceBindingServiceDesc);
}

Loader::~Loader()
{
    
}

}
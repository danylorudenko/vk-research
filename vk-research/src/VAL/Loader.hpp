#pragma once

#include <memory>

#include "..\class_features\NonCopyable.hpp"
#include "..\system\DynamicLibrary.hpp"

#include "ImportTable.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Surface.hpp"
#include "Swapchain.hpp"
#include "memory\MemoryController.hpp"
#include "resources\ResourcesController.hpp"
#include "image\ImagesProvider.hpp"
#include "buffer\BuffersProvider.hpp"
#include "pipeline\DescriptorLayoutController.hpp"
#include "runtime\DescriptorSetController.hpp"
#include "worker\WorkersProvider.hpp"
#include "runtime\FramedDescriptorsHub.hpp"
#include "ResourceRendererProxy.hpp"
#include "runtime\ResourceBindingService.hpp"
#include "pipeline\RenderPassController.hpp"
#include "resources\FramebufferController.hpp"
#include "pipeline\PipelineFactory.hpp"
#include "pipeline\ShaderModuleFactory.hpp"
#include "runtime\PresentationController.hpp"

class IOManager;

namespace VAL
{

struct LoaderDesc
{
    HINSTANCE hInstance_;
    HWND hwnd_;
    std::uint32_t bufferingCount_;
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


    std::unique_ptr<DynamicLibrary> vulkanLibrary_;

    std::unique_ptr<VAL::ImportTable> table_;

    std::unique_ptr<VAL::Instance> instance_;
    std::unique_ptr<VAL::Device> device_;
    std::unique_ptr<VAL::Surface> surface_;
    std::unique_ptr<VAL::Swapchain> swapchain_;

    std::unique_ptr<VAL::MemoryController> memoryController_;
    std::unique_ptr<VAL::ResourcesController> resourcesController_;

    std::unique_ptr<VAL::BuffersProvider> buffersProvider_;
    std::unique_ptr<VAL::ImagesProvider> imagesProvider_;

    std::unique_ptr<VAL::DescriptorLayoutController> descriptorLayoutController_;
    std::unique_ptr<VAL::DescriptorSetController> descriptorSetController_;

    std::unique_ptr<VAL::FramedDescriptorsHub> framedDescriptorsHub_;

    std::unique_ptr<VAL::RenderPassController> renderPassController_;
    std::unique_ptr<VAL::FramebufferController> framebufferController_;

    std::unique_ptr<VAL::ResourceRendererProxy> resourceRendererProxy_;

    std::unique_ptr<VAL::ShaderModuleFactory> shaderModuleFactory_;
    std::unique_ptr<VAL::PipelineFactory> pipelineFactory_;

    std::unique_ptr<VAL::WorkersProvider> workersProvider_;

    std::unique_ptr<VAL::PresentationController> presentationController_;


    //std::unique_ptr<VAL::ResourceBindingService> resourceBindingService_;

};

}
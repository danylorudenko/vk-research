#pragma once

#include <memory>

#include "..\class_features\NonCopyable.hpp"
#include "..\system\DynamicLibrary.hpp"

#include "ImportTable.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Surface.hpp"
#include "Swapchain.hpp"
#include "memory/MemoryController.hpp"
#include "resources/ResourcesController.hpp"
#include "image/ImagesProvider.hpp"
#include "buffer/BuffersProvider.hpp"
#include "worker/WorkersProvider.hpp"
#include "runtime/FramedDescriptorsHub.hpp"
#include "ResourceRendererProxy.hpp"
#include "runtime/ResourceBindingService.hpp"
#include "pipeline/PipelineFactory.hpp"
#include "pipeline/ShaderModuleFactory.hpp"

class IOManager;

namespace VKW
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

    std::unique_ptr<VKW::ImportTable> table_;

    std::unique_ptr<VKW::Instance> instance_;
    std::unique_ptr<VKW::Device> device_;
    std::unique_ptr<VKW::Surface> surface_;
    std::unique_ptr<VKW::Swapchain> swapchain_;

    std::unique_ptr<VKW::WorkersProvider> workersProvider_;

    std::unique_ptr<VKW::MemoryController> memoryController_;
    std::unique_ptr<VKW::ResourcesController> resourcesController_;

    std::unique_ptr<VKW::BuffersProvider> buffersProvider_;
    std::unique_ptr<VKW::ImagesProvider> imagesProvider_;

    std::unique_ptr<VKW::FramedDescriptorsHub> framedDescriptorsHub_;

    std::unique_ptr<VKW::ResourceRendererProxy> resourceRendererProxy_;
    //std::unique_ptr<VKW::ShaderModuleFactory> shaderModuleFactory_;
    //std::unique_ptr<VKW::PipelineFactory> pipelineFactory_;

    //std::unique_ptr<VKW::ResourceBindingService> resourceBindingService_;

};

}
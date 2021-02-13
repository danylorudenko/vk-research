#pragma once

#include <memory>

#include <class_features\NonCopyable.hpp>
#include <system\DynamicLibrary.hpp>

#include <vk_interface\ImportTable.hpp>
#include <vk_interface\Instance.hpp>
#include <vk_interface\Device.hpp>
#include <vk_interface\Surface.hpp>
#include <vk_interface\Swapchain.hpp>
#include <vk_interface\memory\MemoryController.hpp>
#include <vk_interface\resources\ResourcesController.hpp>
#include <vk_interface\image\ImagesProvider.hpp>
#include <vk_interface\buffer\BuffersProvider.hpp>
#include <vk_interface\pipeline\DescriptorLayoutController.hpp>
#include <vk_interface\runtime\DescriptorSetController.hpp>
#include <vk_interface\worker\WorkersProvider.hpp>
#include <vk_interface\runtime\FramedDescriptorsHub.hpp>
#include <vk_interface\pipeline\RenderPassController.hpp>
#include <vk_interface\resources\FramebufferController.hpp>
#include <vk_interface\pipeline\PipelineFactory.hpp>
#include <vk_interface\pipeline\ShaderModuleFactory.hpp>
#include <vk_interface\runtime\PresentationController.hpp>

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

    std::unique_ptr<VKW::MemoryController> memoryController_;
    std::unique_ptr<VKW::ResourcesController> resourcesController_;

    std::unique_ptr<VKW::BuffersProvider> buffersProvider_;
    std::unique_ptr<VKW::ImagesProvider> imagesProvider_;

    std::unique_ptr<VKW::DescriptorLayoutController> descriptorLayoutController_;
    std::unique_ptr<VKW::DescriptorSetController> descriptorSetController_;

    std::unique_ptr<VKW::FramedDescriptorsHub> framedDescriptorsHub_;

    std::unique_ptr<VKW::RenderPassController> renderPassController_;
    std::unique_ptr<VKW::FramebufferController> framebufferController_;

    std::unique_ptr<VKW::ShaderModuleFactory> shaderModuleFactory_;
    std::unique_ptr<VKW::PipelineFactory> pipelineFactory_;

    std::unique_ptr<VKW::WorkersProvider> workersProvider_;

    std::unique_ptr<VKW::PresentationController> presentationController_;


    //std::unique_ptr<VKW::ResourceBindingService> resourceBindingService_;

};

}
#include "VulkanApplicationDelegate.hpp"
#include "..\vk_interface\Tools.hpp"
#include "..\renderer\Root.hpp"

VulkanApplicationDelegate::VulkanApplicationDelegate(HINSTANCE instance, char const* title, std::uint32_t windowWidth, std::uint32_t windowHeight, bool vkDebug)
    : mainWindow_ {
        instance,
        title,
        windowWidth,
        windowHeight,
        "VulkanRenderWindow",
        VulkanApplicationDelegate::WinProc,
        this }
{
    VKW::LoaderDesc loaderDesc;
    loaderDesc.hInstance_ = instance; 
    loaderDesc.hwnd_ = mainWindow_.NativeHandle();
    loaderDesc.bufferingCount_ = 2;
    loaderDesc.ioManager_ = &ioManager_;
    loaderDesc.debug_ = vkDebug;

    vulkanLoader_ = std::make_unique<VKW::Loader>(loaderDesc);



    Render::RootDesc rootDesc;
    rootDesc.resourceProxy_ = vulkanLoader_->resourceRendererProxy_.get();
    rootDesc.renderPassController_ = vulkanLoader_->renderPassController_.get();
    rootDesc.imagesProvider_ = vulkanLoader_->imagesProvider_.get();
    rootDesc.framedDescriptorsHub_ = vulkanLoader_->framedDescriptorsHub_.get();
    rootDesc.layoutController_ = vulkanLoader_->descriptorLayoutController_.get();
    rootDesc.defaultFramebufferWidth_ = windowWidth;
    rootDesc.defaultFramebufferHeight_ = windowHeight;

    renderRoot_ = std::make_unique<Render::Root>(rootDesc);
}

VulkanApplicationDelegate::~VulkanApplicationDelegate()
{

}

LRESULT VulkanApplicationDelegate::WinProc(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
    auto* appDelegate = reinterpret_cast<VulkanApplicationDelegate*>(::GetWindowLongPtr(handle, GWLP_USERDATA));
    
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        break;
    }
    
    return ::DefWindowProc(handle, message, wparam, lparam);
}

void VulkanApplicationDelegate::start()
{
    FakeParseRendererResources();
    
    //auto& device = vulkanLoader_.Device();
    //
    //VKW::BufferCreateInfo buffInfo;
    //buffInfo.size_ = 256;
    //buffInfo.usage_ = VKW::BufferUsage::VERTEX_INDEX;
    //
    //VKW::Buffer buffer = vulkanLoader_.BufferLoader().LoadBuffer(buffInfo);
    //VKW::Buffer buffer2 = vulkanLoader_.BufferLoader().LoadBuffer(buffInfo);
    //
    //\\\\\\\
    //
    //auto* worker = vulkanLoader_.WorkersProvider().GetWorker(VKW::WorkerType::GRAPHICS, 0);
    //auto commandBuffer = worker->StartNextExecutionFrame();
    //
    //
    //VkBufferCopy copyRegion;
    //copyRegion.srcOffset = 0;
    //copyRegion.dstOffset = 0;
    //copyRegion.size = 256;
    //
    //vulkanLoader_.Table().vkCmdCopyBuffer(commandBuffer, buffer.handle_, buffer2.handle_, 1, &copyRegion);
    //
    //worker->EndCurrentExecutionFrame();
    //worker->ExecuteCurrentFrame();
    //
    //vulkanLoader_.Table().vkDeviceWaitIdle(device.Handle());
    //
    //
    //\\\\\\\
    //
    //vulkanLoader_.BufferLoader().UnloadBuffer(buffer);
    //vulkanLoader_.BufferLoader().UnloadBuffer(buffer2);

}

void VulkanApplicationDelegate::update()
{

}

void VulkanApplicationDelegate::shutdown()
{

}

void VulkanApplicationDelegate::FakeParseRendererResources()
{
    VKW::ImageViewDesc imageDesc;
    imageDesc.format_ = VK_FORMAT_R8G8B8A8_UNORM;
    imageDesc.usage_ = VKW::ImageUsage::RENDER_TARGET;
    imageDesc.width_ = 1024;
    imageDesc.height_ = 1024;

    renderRoot_->DefineGlobalImage("attchmnt0", imageDesc);

    renderRoot_->DefineGlobalImage("attchmnt1", imageDesc);

    renderRoot_->DefineGlobalImage("attchmnt2", imageDesc);




    Render::RootPassDesc passDesc;
    passDesc.colorAttachmentsCount_ = 2;
    passDesc.colorAttachments_[0] = "attchmnt0";
    passDesc.colorAttachments_[1] = "attchmnt1";
    passDesc.depthStencilAttachment_ = "";

    renderRoot_->DefineRenderPass("pass0", passDesc);




    VKW::DescriptorSetLayoutDesc setLayoutDesc;
    setLayoutDesc.membersCount_ = 3;

    setLayoutDesc.membersDesc_[0].type_ = VKW::DESCRIPTOR_TYPE_SAMPLED_TEXTURE;
    setLayoutDesc.membersDesc_[0].binding_ = 0;

    setLayoutDesc.membersDesc_[1].type_ = VKW::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setLayoutDesc.membersDesc_[1].binding_ = 1;
    
    setLayoutDesc.membersDesc_[2].type_ = VKW::DESCRIPTOR_TYPE_SAMPLER;
    setLayoutDesc.membersDesc_[2].binding_ = 2;

    renderRoot_->DefineSetLayout("layout0", setLayoutDesc);




    VKW::GraphicsPipelineDesc pipelineDesc;

    VKW::VertexInputInfo vInfo;
    vInfo.binding_ = 0;
    //vInfo.

}

#include "VulkanApplicationDelegate.hpp"
#include "..\vk_interface\Tools.hpp"
#include "..\renderer\Root.hpp"

#include <utility>

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
    rootDesc.loader_ = vulkanLoader_.get();
    rootDesc.resourceProxy_ = vulkanLoader_->resourceRendererProxy_.get();
    rootDesc.renderPassController_ = vulkanLoader_->renderPassController_.get();
    rootDesc.imagesProvider_ = vulkanLoader_->imagesProvider_.get();
    rootDesc.framedDescriptorsHub_ = vulkanLoader_->framedDescriptorsHub_.get();
    rootDesc.layoutController_ = vulkanLoader_->descriptorLayoutController_.get();
    rootDesc.shaderModuleFactory_ = vulkanLoader_->shaderModuleFactory_.get();
    rootDesc.pipelineFactory_ = vulkanLoader_->pipelineFactory_.get();
    rootDesc.presentationController_ = vulkanLoader_->presentationController_.get();
    rootDesc.mainWorkerTemp_ = vulkanLoader_->workersProvider_->GetWorker(VKW::WorkerType::GRAPHICS_PRESENT, 0);
    rootDesc.defaultFramebufferWidth_ = vulkanLoader_->swapchain_->Width();
    rootDesc.defaultFramebufferHeight_ = vulkanLoader_->swapchain_->Height();

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
    //auto commandBuffer = worker->StartExecutionFrame();
    //
    //
    //VkBufferCopy copyRegion;
    //copyRegion.srcOffset = 0;
    //copyRegion.dstOffset = 0;
    //copyRegion.size = 256;
    //
    //vulkanLoader_.Table().vkCmdCopyBuffer(commandBuffer, buffer.handle_, buffer2.handle_, 1, &copyRegion);
    //
    //worker->EndExecutionFrame();
    //worker->ExecuteFrame();
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
    renderRoot_->IterateRenderGraph();
}

void VulkanApplicationDelegate::shutdown()
{

}

void VulkanApplicationDelegate::FakeParseRendererResources()
{
    //VKW::DescriptorSetLayoutDesc setLayoutDesc;
    //setLayoutDesc.membersCount_ = 3;
    //
    //setLayoutDesc.membersDesc_[0].type_ = VKW::DESCRIPTOR_TYPE_SAMPLED_TEXTURE;
    //setLayoutDesc.membersDesc_[0].binding_ = 0;
    //
    //setLayoutDesc.membersDesc_[1].type_ = VKW::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //setLayoutDesc.membersDesc_[1].binding_ = 1;
    //
    //setLayoutDesc.membersDesc_[2].type_ = VKW::DESCRIPTOR_TYPE_SAMPLER;
    //setLayoutDesc.membersDesc_[2].binding_ = 2;
    //
    //renderRoot_->DefineSetLayout("layout0", setLayoutDesc);


    //VKW::ImageViewDesc imageDesc;
    //imageDesc.format_ = VK_FORMAT_R8G8B8A8_UNORM;
    //imageDesc.usage_ = VKW::ImageUsage::RENDER_TARGET;
    //imageDesc.width_ = 1024;
    //imageDesc.height_ = 1024;
    //
    //renderRoot_->DefineGlobalImage("attchmnt0", imageDesc);

    //renderRoot_->DefineGlobalImage("attchmnt1", imageDesc);

    //renderRoot_->DefineGlobalImage("attchmnt2", imageDesc);


    Render::RootPassDesc passDesc;
    passDesc.colorAttachmentsCount_ = 1;
    passDesc.colorAttachments_[0] = Render::Root::SWAPCHAIN_IMAGE_KEY; // we need swapchain reference here

    renderRoot_->DefineRenderPass("pass0", passDesc);
    Render::Pass& pass = renderRoot_->FindPass("pass0");

    VKW::ShaderModuleDesc vertexModuleDesc;
    vertexModuleDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_VERTEX;
    vertexModuleDesc.shaderPath_ = "shader-src\\test-vertex.spv";
    vertexModuleDesc.entryPoint_ = "main";


    VKW::ShaderModuleDesc fragmentModuleDesc;
    fragmentModuleDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_FRAGMENT;
    fragmentModuleDesc.shaderPath_ = "shader-src\\test-frag.spv";
    fragmentModuleDesc.entryPoint_ = "main";
    
    Render::RootPipelineDesc pipelineDesc;

    pipelineDesc.renderPass_ = "pass0";
    pipelineDesc.shaderStagesCount_ = 2;
    pipelineDesc.shaderStages_[0].desc_ = std::move(vertexModuleDesc);
    pipelineDesc.shaderStages_[1].desc_ = std::move(fragmentModuleDesc);

    VKW::InputAssemblyInfo iaInfo;
    iaInfo.primitiveRestartEnable_ = false;
    iaInfo.primitiveTopology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VKW::VertexInputInfo vInfo;
    vInfo.binding_ = 0;
    vInfo.stride_ = 0;
    vInfo.vertexAttributesCount_ = 0;
    //vInfo.vertexAttributes_[0].location_ = 0;
    //vInfo.vertexAttributes_[0].offset_ = 0;
    //vInfo.vertexAttributes_[0].format_ = VK_FORMAT_R32G32B32_SFLOAT;

    auto const width = (mainWindow_.Width());
    auto const height = (mainWindow_.Height());

    VKW::ViewportInfo vpInfo;
    vpInfo.viewportsCount_ = 1;
    auto& vp = vpInfo.viewports_[0];
    vp.x_ = 0.0f;
    vp.y_ = 0.0f;
    vp.width_ = static_cast< float >(width);
    vp.height_ = static_cast< float >(height);
    vp.minDepth_ = 0.0f;
    vp.maxDepth_ = 1.0f;
    vp.scissorXoffset_ = 0;
    vp.scissorYoffset_ = 0;
    vp.scissorXextent_ = width;
    vp.scissorYextent_ = height;

    VKW::PipelineLayoutDesc layoutDesc;
    layoutDesc.membersCount_ = 0;

    pipelineDesc.inputAssemblyInfo_ = &iaInfo;
    pipelineDesc.vertexInputInfo_ = &vInfo;
    pipelineDesc.viewportInfo_ = &vpInfo;
    pipelineDesc.layoutDesc_ = &layoutDesc;

    renderRoot_->DefineGraphicsPipeline("testpipe", pipelineDesc);

    pass.AddPipeline("testpipe");
    renderRoot_->PushPassTemp("pass0");

}

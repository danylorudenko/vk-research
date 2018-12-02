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
    rootDesc.pipelineFactory_ = vulkanLoader_->pipelineFactory_.get();
    rootDesc.presentationController_ = vulkanLoader_->presentationController_.get();
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


    VKW::ImageViewDesc imageDesc;
    imageDesc.format_ = VK_FORMAT_R8G8B8A8_UNORM;
    imageDesc.usage_ = VKW::ImageUsage::RENDER_TARGET;
    imageDesc.width_ = 1024;
    imageDesc.height_ = 1024;

    renderRoot_->DefineGlobalImage("attchmnt0", imageDesc);

    //renderRoot_->DefineGlobalImage("attchmnt1", imageDesc);

    //renderRoot_->DefineGlobalImage("attchmnt2", imageDesc);


    Render::RootPassDesc passDesc;
    passDesc.colorAttachmentsCount_ = 1;
    passDesc.colorAttachments_[0] = "attchmnt0";

    renderRoot_->DefineRenderPass("pass0", passDesc);

    VKW::ShaderModuleDesc vertexModuleDesc;
    vertexModuleDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_VERTEX;
    vertexModuleDesc.shaderPath_ = "shader-src\\test-vertex.spv";
    vertexModuleDesc.entryPoint_ = "main";


    VKW::ShaderModuleDesc fragmentModuleDesc;
    fragmentModuleDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_FRAGMENT;
    fragmentModuleDesc.shaderPath_ = "shader-src\\test-frag.spv";
    fragmentModuleDesc.entryPoint_ = "main";

    VKW::ShaderModuleHandle vertexHandle = vulkanLoader_->shaderModuleFactory_->LoadModule(vertexModuleDesc);
    VKW::ShaderModuleHandle fragmentHandle = vulkanLoader_->shaderModuleFactory_->LoadModule(fragmentModuleDesc);




    Render::RootPipelineDesc pipelineDesc;


    pipelineDesc.shaderStagesCount_ = 2;
    pipelineDesc.shaderStages_[0].shaderModuleHandle_ = vertexHandle;
    pipelineDesc.shaderStages_[1].shaderModuleHandle_ = fragmentHandle;



    VKW::InputAssemblyInfo iaInfo;
    iaInfo.primitiveRestartEnable_ = false;
    iaInfo.primitiveTopology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    pipelineDesc.inputAssemblyInfo_ = &iaInfo;




    VKW::VertexInputInfo vInfo;
    vInfo.binding_ = 0;
    vInfo.stride_ = 12;
    vInfo.vertexAttributesCount_ = 1;
    vInfo.vertexAttributes_[0].location_ = 0;
    vInfo.vertexAttributes_[0].offset_ = 0;
    vInfo.vertexAttributes_[0].format_ = VK_FORMAT_R32G32B32_SFLOAT;

    pipelineDesc.vertexInputInfo_ = &vInfo;



    VKW::ViewportInfo vpInfo;
    vpInfo.viewportsCount_ = 1;

    auto& vp = vpInfo.viewports_[0];
    vp.x_ = 0.0f;
    vp.y_ = 0.0f;
    vp.width_ = 1024.0f;
    vp.height_ = 1024.0f;
    vp.minDepth_ = 0.0f;
    vp.maxDepth_ = 1.0f;
    vp.scissorXoffset_ = 0;
    vp.scissorYoffset_ = 0;
    vp.scissorXextent_ = 1024;
    vp.scissorYextent_ = 1024;

    pipelineDesc.viewportInfo_ = &vpInfo;



    VKW::PipelineLayoutDesc layoutDesc;
    layoutDesc.membersCount_ = 0;

    pipelineDesc.layoutDesc_ = &layoutDesc;

    pipelineDesc.renderPass_ = "pass0";


    renderRoot_->DefineGraphicsPipeline("testpipe", pipelineDesc);

    
    renderRoot_->PushPassTemp("pass0");

}

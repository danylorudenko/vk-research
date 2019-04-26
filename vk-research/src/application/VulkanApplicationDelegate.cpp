#include "VulkanApplicationDelegate.hpp"
#include "..\vk_interface\Tools.hpp"
#include "..\renderer\Root.hpp"

#include "..\renderer\Material.hpp"

#include <utility>
#include <chrono>

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
}

float testcounter = 0.0f;

std::chrono::high_resolution_clock::time_point prevTime;

void VulkanApplicationDelegate::update()
{
    //auto currTime = std::chrono::high_resolution_clock::now();
    //auto frameTime = currTime - prevTime;
    //prevTime = currTime;
    //std::cout << std::chrono::duration_cast<std::chrono::microseconds>(frameTime).count() << std::endl;


    VKW::PresentationContext presentationContext = renderRoot_->AcquireNextPresentationContext();
    std::uint32_t context = presentationContext.contextId_;
    ////////////////////////////////////////////////////
    //

    Render::UniformBufferWriterProxy& proxy = customData_.uniformProxy_;

    void* ptr = proxy.IsMapped(context) ? proxy.MappedPtr(context) : proxy.MapForWrite(context);

    struct TestUniform {
        float x;
        float y;
        float z;
    } uniformData;

    testcounter += testcounter < 2.0f ? 0.001f : -4.0f;
    uniformData.x = testcounter;
    uniformData.y = 0.0f;
    uniformData.z = 0.0f;

    std::memcpy(ptr, &uniformData, sizeof(TestUniform));

    proxy.Flush(context);

    //
    ////////////////////////////////////////////////////
    renderRoot_->IterateRenderGraph(presentationContext);
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

    char constexpr uploadBufferKey[] = "uppl0";
    char constexpr vertexBufferKey[] = "vert0";
    char constexpr passKey[] = "pass0";
    char constexpr pipeKey[] = "pipe0";
    char constexpr setLayoutKey[] = "set0";
    char constexpr materialTemplateKey[] = "templ0";
    char constexpr materialKey[] = "mat0";

    struct TestVertex
    {
        float x;
        float y;
        float z;
    };

    TestVertex vertexData[3] = {
        { -0.5, 0.5, 0.0 },
        { 0.5, 0.5, 0.0 },
        { 0.0, -0.5, 0.0 }
    };

    VKW::BufferViewDesc bufferDecs;
    bufferDecs.format_ = VK_FORMAT_UNDEFINED;
    bufferDecs.size_ = sizeof(vertexData);
    bufferDecs.usage_ = VKW::BufferUsage::UPLOAD_BUFFER;
    renderRoot_->DefineGlobalBuffer(uploadBufferKey, bufferDecs);

    bufferDecs.usage_ = VKW::BufferUsage::VERTEX_INDEX;
    renderRoot_->DefineGlobalBuffer(vertexBufferKey, bufferDecs);


    void* mappedUploadBuffer = renderRoot_->MapBuffer(uploadBufferKey, 0);
    std::memcpy(mappedUploadBuffer, vertexData, sizeof(vertexData));
    renderRoot_->FlushBuffer(uploadBufferKey, 0);

    renderRoot_->CopyBuffer(uploadBufferKey, vertexBufferKey, 0);



    Render::RenderPassDesc passDesc;
    passDesc.colorAttachmentsCount_ = 1;
    passDesc.colorAttachments_[0] = Render::Root::SWAPCHAIN_IMAGE_KEY; // we need swapchain reference here

    renderRoot_->DefineRenderPass(passKey, passDesc);
    Render::Pass& pass = renderRoot_->FindPass(passKey);

    Render::ShaderDesc vertexShaderDesc;
    vertexShaderDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_VERTEX;
    vertexShaderDesc.relativePath_ = "shader-src\\test-vertex.spv";
    
    Render::ShaderDesc fragmentShaderDesc;
    fragmentShaderDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_FRAGMENT;
    fragmentShaderDesc.relativePath_ = "shader-src\\test-frag.spv";

    renderRoot_->DefineShader("vShader", vertexShaderDesc);
    renderRoot_->DefineShader("fShader", fragmentShaderDesc);

    Render::GraphicsPipelineDesc pipelineDesc;

    pipelineDesc.renderPass_ = passKey;
    pipelineDesc.shaderStagesCount_ = 2;
    pipelineDesc.shaderStages_[0] = "vShader";
    pipelineDesc.shaderStages_[1] = "fShader";

    VKW::InputAssemblyInfo iaInfo;
    iaInfo.primitiveRestartEnable_ = false;
    iaInfo.primitiveTopology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VKW::VertexInputInfo vInfo;
    vInfo.binding_ = 0;
    vInfo.stride_ = sizeof(TestVertex);
    vInfo.vertexAttributesCount_ = 1;
    vInfo.vertexAttributes_[0].location_ = 0;
    vInfo.vertexAttributes_[0].offset_ = 0;
    vInfo.vertexAttributes_[0].format_ = VK_FORMAT_R32G32B32_SFLOAT;

    std::uint32_t const width = (mainWindow_.Width());
    std::uint32_t const height = (mainWindow_.Height());

    VKW::ViewportInfo vpInfo;
    vpInfo.viewportsCount_ = 1;
    VKW::ViewportInfo::Viewport& vp = vpInfo.viewports_[0];
    vp.x_ = 0.0f;
    vp.y_ = 0.0f;
    vp.width_ = static_cast<float>(width);
    vp.height_ = static_cast<float>(height);
    vp.minDepth_ = 0.0f;
    vp.maxDepth_ = 1.0f;
    vp.scissorXoffset_ = 0;
    vp.scissorYoffset_ = 0;
    vp.scissorXextent_ = width;
    vp.scissorYextent_ = height;

    VKW::DescriptorSetLayoutDesc setLayoutDesc;
    setLayoutDesc.membersCount_ = 1;
    setLayoutDesc.membersDesc_[0].type_ = VKW::DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setLayoutDesc.membersDesc_[0].binding_ = 0;
    renderRoot_->DefineSetLayout(setLayoutKey, setLayoutDesc);

    Render::PipelineLayoutDesc layoutDesc;
    layoutDesc.staticMembersCount_ = 0;
    layoutDesc.instancedMembersCount_ = 1;
    layoutDesc.instancedMembers_[0] = setLayoutKey;

    pipelineDesc.inputAssemblyInfo_ = &iaInfo;
    pipelineDesc.vertexInputInfo_ = &vInfo;
    pipelineDesc.viewportInfo_ = &vpInfo;
    pipelineDesc.layoutDesc_ = &layoutDesc;

    renderRoot_->DefineGraphicsPipeline(pipeKey, pipelineDesc);
    
    
    Render::MaterialTemplateDesc materialTemplateDesc;
    materialTemplateDesc.perPassDataCount_ = 1;
    materialTemplateDesc.perPassData_[0].passKey_ = passKey;
    materialTemplateDesc.perPassData_[0].pipelineKey_ = pipeKey;
    renderRoot_->DefineMaterialTemplate(materialTemplateKey, materialTemplateDesc);

    Render::MaterialDesc materialDesc;
    materialDesc.templateKey_ = materialTemplateKey;
    renderRoot_->DefineMaterial(materialKey, materialDesc);


    Render::RenderItemDesc itemDesc;
    itemDesc.vertexBufferKey_ = vertexBufferKey;
    itemDesc.vertexCount_ = 3;
    itemDesc.setOwnerDescs_[0].members_[0].uniformBuffer_.size_ = 64;

    Render::RenderWorkItemHandle renderItemHandle = renderRoot_->ConstructRenderWorkItem(pipeKey, itemDesc);
    Render::RenderWorkItem* item = renderRoot_->FindRenderWorkItem(pipeKey, renderItemHandle);
    customData_.uniformProxy_ = Render::UniformBufferWriterProxy(renderRoot_.get(), item, 0, 0);
    customData_.uniformProxy_.MapForWrite(0);
    // TODO
    //customData_.uniformProxy_.MapForWrite(1);

    renderRoot_->RegisterMaterial(materialKey);
    renderRoot_->PushPassTemp(passKey);

    // how to write resource descriptor to DescriptorSet
    //renderRoot_->Defue

}

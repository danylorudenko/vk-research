#include "VulkanApplicationDelegate.hpp"
#include "..\vk_interface\Tools.hpp"
#include "..\renderer\Root.hpp"

#include "..\renderer\Material.hpp"
#include "..\transform\TansformComponent.hpp"

#include <utility>
#include <chrono>

#include <glm\gtc\quaternion.hpp>

#include <imgui/imgui.h>

VulkanApplicationDelegate::VulkanApplicationDelegate(HINSTANCE instance, char const* title, std::uint32_t windowWidth, std::uint32_t windowHeight, std::uint32_t buffering, bool vkDebug, bool imguiEnabled)
    : mainWindow_ {
        instance,
        title,
        windowWidth,
        windowHeight,
        "VulkanRenderWindow",
        VulkanApplicationDelegate::WinProc,
        this }
    , imguiEnabled_{ imguiEnabled }
{
    VKW::LoaderDesc loaderDesc;
    loaderDesc.hInstance_ = instance; 
    loaderDesc.hwnd_ = mainWindow_.NativeHandle();
    loaderDesc.bufferingCount_ = buffering;
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

    ImGuiHelperDesc imguiHelperDesc;
    imguiHelperDesc.root_ = renderRoot_.get();
    imguiHelper_ = std::make_unique<ImGuiHelper>(imguiHelperDesc);
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
    if (imguiEnabled_)
        InitImGui();

    FakeParseRendererResources();
}

//std::chrono::high_resolution_clock::time_point prevTime;
float testcounter = 0.0f;


void VulkanApplicationDelegate::update()
{
    ////////////////////////////////////////////////////
    // Frame time measurement
    //auto currTime = std::chrono::high_resolution_clock::now();
    //auto frameTime = currTime - prevTime;
    //prevTime = currTime;
    //std::cout << std::chrono::duration_cast<std::chrono::microseconds>(frameTime).count() << std::endl;


    VKW::PresentationContext presentationContext = renderRoot_->AcquireNextPresentationContext();
    std::uint32_t context = presentationContext.contextId_;
    ////////////////////////////////////////////////////
    //
    if (imguiEnabled_)
        ImGui::NewFrame();


    testcounter += 0.01f;
    customData_.transformComponent_->scale_ = glm::vec3(3.0f);
    customData_.transformComponent_->position_ = glm::vec3(0.0f, 0.5f, -1.0f);
    //customData_.transformComponent_->orientation_.z = glm::degrees(testcounter);
    customData_.transformComponent_->orientation_.z = glm::degrees(3.14f);
    customData_.transformComponent_->orientation_.y = glm::degrees(testcounter * 1.1f);
    

    //
    ////////////////////////////////////////////////////
    transformationSystem_.Update(context, glm::vec3(0.0f), glm::vec3(0.0f), 60.0f);
    renderRoot_->IterateRenderGraph(presentationContext);


    /////////////////////

    if (imguiEnabled_) {
        ImGui::EndFrame();
        ImGui::Render();
        ImDrawData* imguiDrawData = ImGui::GetDrawData();
    }
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
    char constexpr indexBufferKey[] = "ind0";
    char constexpr passKey[] = "pass0";
    char constexpr pipeKey[] = "pipe0";
    char constexpr setLayoutKey[] = "set0";
    char constexpr materialTemplateKey[] = "templ0";
    char constexpr materialKey[] = "mat0";
    char constexpr depthBufferKey[] = "dpth0";

    Data::ModelMesh testMesh = ioManager_.ReadModelMesh("LFS\\dragon.model");

    struct TestVertex
    {
        float x;
        float y;
        float z;

        float nx;
        float ny;
        float nz;
    };

    std::uint32_t const vertexDataSizeBytes = static_cast<std::uint32_t>(testMesh.vertexData_.size());
    std::uint32_t const indexDataSizeBytes = static_cast<std::uint32_t>(testMesh.indexData_.size());
    std::uint32_t const requiredUploadBufferSize = vertexDataSizeBytes > indexDataSizeBytes ? vertexDataSizeBytes : indexDataSizeBytes;

    VKW::BufferViewDesc bufferDecs;
    bufferDecs.format_ = VK_FORMAT_UNDEFINED;
    bufferDecs.size_ = requiredUploadBufferSize;
    bufferDecs.usage_ = VKW::BufferUsage::UPLOAD_BUFFER;
    renderRoot_->DefineGlobalBuffer(uploadBufferKey, bufferDecs);

    bufferDecs.size_ = vertexDataSizeBytes;
    bufferDecs.usage_ = VKW::BufferUsage::VERTEX_INDEX;
    renderRoot_->DefineGlobalBuffer(vertexBufferKey, bufferDecs);

    bufferDecs.size_ = indexDataSizeBytes;
    renderRoot_->DefineGlobalBuffer(indexBufferKey, bufferDecs);


    void* mappedUploadBuffer = renderRoot_->MapBuffer(uploadBufferKey, 0);
    std::memcpy(mappedUploadBuffer, testMesh.vertexData_.data(), vertexDataSizeBytes);
    renderRoot_->FlushBuffer(uploadBufferKey, 0);
    renderRoot_->CopyStagingBufferToGPUBuffer(uploadBufferKey, vertexBufferKey, 0);

    std::memcpy(mappedUploadBuffer, testMesh.indexData_.data(), indexDataSizeBytes);
    renderRoot_->FlushBuffer(uploadBufferKey, 0);
    renderRoot_->CopyStagingBufferToGPUBuffer(uploadBufferKey, indexBufferKey, 0);


    VKW::ImageViewDesc depthBufferDesc;
    depthBufferDesc.usage_ = VKW::ImageUsage::DEPTH;
    depthBufferDesc.format_ = VK_FORMAT_D16_UNORM;
    depthBufferDesc.width_ = mainWindow_.Width();
    depthBufferDesc.height_ = mainWindow_.Height();
    renderRoot_->DefineGlobalImage(depthBufferKey, depthBufferDesc);

    Render::RenderPassDesc passDesc;
    passDesc.colorAttachmentsCount_ = 1;
    passDesc.colorAttachments_[0] = Render::Root::SWAPCHAIN_IMAGE_KEY; // we need swapchain reference here
    passDesc.depthStencilAttachment_ = depthBufferKey;

    renderRoot_->DefineRenderPass(passKey, passDesc);

    Render::ShaderDesc vertexShaderDesc;
    vertexShaderDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_VERTEX;
    vertexShaderDesc.relativePath_ = "shader-src\\test-vertex.spv";
    
    Render::ShaderDesc fragmentShaderDesc;
    fragmentShaderDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_FRAGMENT;
    fragmentShaderDesc.relativePath_ = "shader-src\\test-frag-uniform.spv";

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
    vInfo.vertexAttributesCount_ = 2;
    vInfo.vertexAttributes_[0].location_ = 0;
    vInfo.vertexAttributes_[0].offset_ = 0;
    vInfo.vertexAttributes_[0].format_ = VK_FORMAT_R32G32B32_SFLOAT;
    vInfo.vertexAttributes_[1].location_ = 1;
    vInfo.vertexAttributes_[1].offset_ = sizeof(float) * 3;
    vInfo.vertexAttributes_[1].format_ = VK_FORMAT_R32G32B32_SFLOAT;

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

    VKW::DepthStencilInfo dsInfo;
    dsInfo.depthTestEnabled_ = true;
    dsInfo.depthWriteEnabled_ = true;
    dsInfo.depthCompareOp_ = VK_COMPARE_OP_LESS;
    dsInfo.stencilTestEnabled_ = false;
    dsInfo.backStencilState_ = {};
    dsInfo.frontStencilState_ = {};

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
    pipelineDesc.depthStencilInfo_ = &dsInfo;
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


    Render::RenderWorkItemDesc itemDesc;
    itemDesc.vertexBufferKey_ = vertexBufferKey;
    itemDesc.indexBufferKey_ = indexBufferKey;
    itemDesc.vertexCount_ = vertexDataSizeBytes / sizeof(TestVertex);
    itemDesc.indexCount_ = indexDataSizeBytes / sizeof(std::uint32_t);

    itemDesc.setOwnerDescs_[0].members_[0].uniformBuffer_.size_ = 64;

    Render::RenderWorkItemHandle renderItemHandle = renderRoot_->ConstructRenderWorkItem(pipeKey, itemDesc);
    Render::RenderWorkItem* item = renderRoot_->FindRenderWorkItem(pipeKey, renderItemHandle);
    customData_.uniformProxy_ = Render::UniformBufferWriterProxy(renderRoot_.get(), item, 0, 0);


    renderRoot_->RegisterMaterial(materialKey);
    renderRoot_->PushPassTemp(passKey);

    customData_.transformComponent_ = transformationSystem_.CreateTransformComponent(nullptr, &customData_.uniformProxy_);
    // how to write resource descriptor to DescriptorSet
    //renderRoot_->Defue

}

void VulkanApplicationDelegate::InitImGui()
{
    imguiHelper_->Init(mainWindow_.Width(), mainWindow_.Height());
}

void VulkanApplicationDelegate::RenderImGui(std::uint32_t context)
{
    
}

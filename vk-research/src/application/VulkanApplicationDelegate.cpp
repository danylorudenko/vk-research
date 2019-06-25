#include "VulkanApplicationDelegate.hpp"
#include "..\vk_interface\Tools.hpp"
#include "..\renderer\Root.hpp"

#include "..\renderer\Material.hpp"
#include "..\transform\TansformComponent.hpp"

#include "..\renderer\CustomTempBlurPass.hpp"

#include <utility>
#include <cstdio>

#include <glm\gtc\quaternion.hpp>

#include <imgui/imgui.h>
#include "ImGuiUserData.hpp"

VulkanApplicationDelegate::VulkanApplicationDelegate(HINSTANCE instance, char const* title, std::uint32_t windowWidth, std::uint32_t windowHeight, std::uint32_t buffering, bool vkDebug, bool imguiEnabled)
    : mainWindow_ {
        instance,
        title,
        windowWidth,
        windowHeight,
        "VulkanRenderWindow",
        VulkanApplicationDelegate::WinProc,
        this }
    , inputSystem_{ mainWindow_.NativeHandle() }
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

    renderRoot_ = std::make_unique<Render::Root>(rootDesc);

    ImGuiHelperDesc imguiHelperDesc;
    imguiHelperDesc.window_ = &mainWindow_;
    imguiHelperDesc.inputSystem_ = &inputSystem_;
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
    case WM_INPUT:
    {
        UINT code = GET_RAWINPUT_CODE_WPARAM(wparam);
        LRESULT result;
        if (code == RIM_INPUTSINK || code == RIM_INPUT)
            result = ::DefWindowProc(handle, message, wparam, lparam);

        appDelegate->GetInputSystem().ProcessSystemInput(handle, wparam, lparam);
        return result;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        break;
    }
    
    return ::DefWindowProc(handle, message, wparam, lparam);
}

InputSystem& VulkanApplicationDelegate::GetInputSystem()
{
    return inputSystem_;
}

void VulkanApplicationDelegate::start()
{
    FakeParseRendererResources();

    if (imguiEnabled_)
        InitImGui();
}

float testcounter = 0.0f;


void VulkanApplicationDelegate::update()
{
    ////////////////////////////////////////////////////
    // Frame time measurement
    auto currTime = std::chrono::high_resolution_clock::now();
    auto frameTime = currTime - prevFrameTimePoint_;
    prevFrameTimePoint_ = currTime;
    prevFrameDelta_ = std::chrono::duration_cast<std::chrono::milliseconds>(frameTime).count();


    inputSystem_.Update();

    VKW::PresentationContext presentationContext = renderRoot_->AcquireNextPresentationContext();
    std::uint32_t context = presentationContext.contextId_;
    ////////////////////////////////////////////////////
    //
    if (imguiEnabled_)
        imguiHelper_->BeginFrame(context);


    testcounter += 0.01f;

    for (std::uint32_t x = 0; x < 5; ++x) {
        for (std::uint32_t y = 0; y < 5; ++y) {
            customData_.transformComponents_[x * 5 + y]->scale_ = glm::vec3(5.0f);
            customData_.transformComponents_[x * 5 + y]->position_ = glm::vec3(-2.0f + x, -1.8f + y, -3.1f);
            customData_.transformComponents_[x * 5 + y]->orientation_.z = glm::degrees(3.14f);
            customData_.transformComponents_[x * 5 + y]->orientation_.y = glm::degrees(testcounter * 1.1f);
        }
    }
    

    //
    ////////////////////////////////////////////////////
    Transform::TransformSystemCameraData cameraData;
    cameraData.cameraPos = glm::vec3(0.0f);
    cameraData.cameraEuler = glm::vec3(0.0f);
    cameraData.cameraFowDegrees = 60.0f;

    VKW::ImageView* colorBufferView = renderRoot_->FindGlobalImage(renderRoot_->GetDefaultSceneColorOutput(), 0);
    VKW::ImageResource* colorBufferResource = renderRoot_->ResourceProxy()->GetResource(colorBufferView->resource_);
    cameraData.width = (float)colorBufferResource->width_;
    cameraData.height = (float)colorBufferResource->height_;

    transformationSystem_.Update(context, cameraData);

    VKW::WorkerFrameCommandReciever commandReciever = renderRoot_->BeginRenderGraph(presentationContext);
    renderRoot_->IterateRenderGraph(presentationContext, commandReciever);

    ImGuiUser(context);


    if (imguiEnabled_) {
        imguiHelper_->EndFrame(context);
        imguiHelper_->Render(context, commandReciever);
    }

    renderRoot_->EndRenderGraph(presentationContext, commandReciever);
}

void VulkanApplicationDelegate::shutdown()
{
    
}

void VulkanApplicationDelegate::FakeParseRendererResources()
{
    char constexpr uploadBufferKey[] = "uppl0";
    char constexpr vertexBufferKey[] = "vert0";
    char constexpr indexBufferKey[] = "ind0";
    char constexpr passKey[] = "pass0";
    char constexpr pipeKey[] = "pipe0";
    char constexpr backgroundPipeKey[] = "pipe1";
    char constexpr setLayoutKey[] = "set0";
    char constexpr backgroundSetLayoutKey[] = "set1";
    char constexpr materialTemplateKey[] = "templ0";
    char constexpr backgroundMaterialTemplateKey[] = "templ1";
    char constexpr materialKey[] = "mat0";
    char constexpr backgroundMaterialKey[] = "mat1";
    char constexpr depthBufferKey[] = "dpth0";

    char constexpr blurPass[] = "blur";

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



    VKW::ImageView* colorBufferView = renderRoot_->FindGlobalImage(renderRoot_->GetDefaultSceneColorOutput(), 0);
    VKW::ImageResource* colorBufferResource = renderRoot_->ResourceProxy()->GetResource(colorBufferView->resource_);

    VKW::ImageViewDesc depthBufferDesc;
    depthBufferDesc.usage_ = VKW::ImageUsage::DEPTH;
    depthBufferDesc.format_ = VK_FORMAT_D16_UNORM;
    depthBufferDesc.width_ = colorBufferResource->width_;
    depthBufferDesc.height_ = colorBufferResource->height_;
    renderRoot_->DefineGlobalImage(depthBufferKey, depthBufferDesc);

    Render::RootGraphicsPassDesc passDesc;
    passDesc.colorAttachmentsCount_ = 1;
    passDesc.colorAttachments_[0].resourceKey_ = renderRoot_->GetDefaultSceneColorOutput();
    passDesc.colorAttachments_[0].usage_ = VKW::RENDER_PASS_ATTACHMENT_USAGE_COLOR_CLEAR;
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

    std::uint32_t const width = (colorBufferResource->width_);
    std::uint32_t const height = (colorBufferResource->height_);

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
    setLayoutDesc.stage_ = VKW::DescriptorStage::RENDERING;
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
    pipelineDesc.dynamicStateFlags_ = VK_FLAGS_NONE;
    pipelineDesc.blendingState_ = VKW::PIPELINE_BLENDING_NONE;

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
    itemDesc.vertexBindOffset_ = 0;
    itemDesc.indexCount_ = indexDataSizeBytes / sizeof(std::uint32_t);
    itemDesc.indexBindOffset_ = 0;

    itemDesc.setOwnerDescs_[0].members_[0].uniformBuffer_.size_ = 64;


    for (std::uint32_t i = 0; i < 25; ++i) {
        Render::RenderWorkItemHandle renderItemHandle = renderRoot_->ConstructRenderWorkItem(pipeKey, itemDesc);

        Render::RenderWorkItem* item = renderRoot_->FindRenderWorkItem(pipeKey, renderItemHandle);
        customData_.uniformProxies[i] = Render::UniformBufferWriterProxy(renderRoot_.get(), item, 0, 0);

        customData_.transformComponents_[i] = transformationSystem_.CreateTransformComponent(nullptr, &customData_.uniformProxies[i]);
    }
    


    renderRoot_->RegisterMaterial(materialKey);
    renderRoot_->PushPass(passKey);


    renderRoot_->DefineCustomBlurPass(blurPass, renderRoot_->GetDefaultSceneColorOutput(), &ioManager_);
    renderRoot_->PushPass(blurPass);



    // background
    Render::ShaderDesc backgroundVertexShaderDesc;
    backgroundVertexShaderDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_VERTEX;
    backgroundVertexShaderDesc.relativePath_ = "shader-src\\background.vert.spv";

    Render::ShaderDesc backgroundFragmentShaderDesc;
    backgroundFragmentShaderDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_FRAGMENT;
    backgroundFragmentShaderDesc.relativePath_ = "shader-src\\background.frag.spv";

    renderRoot_->DefineShader("bvShader", backgroundVertexShaderDesc);
    renderRoot_->DefineShader("bfShader", backgroundFragmentShaderDesc);



    Render::GraphicsPipelineDesc pipelineBackroundDesc;

    pipelineBackroundDesc.renderPass_ = passKey;
    pipelineBackroundDesc.shaderStagesCount_ = 2;
    pipelineBackroundDesc.shaderStages_[0] = "bvShader";
    pipelineBackroundDesc.shaderStages_[1] = "bfShader";


    VKW::DescriptorSetLayoutDesc backgroundSetLayoutDesc;
    backgroundSetLayoutDesc.stage_ = VKW::DescriptorStage::RENDERING;
    backgroundSetLayoutDesc.membersCount_ = 1;
    backgroundSetLayoutDesc.membersDesc_[0].type_ = VKW::DESCRIPTOR_TYPE_TEXTURE;
    backgroundSetLayoutDesc.membersDesc_[0].binding_ = 0;
    renderRoot_->DefineSetLayout(backgroundSetLayoutKey, backgroundSetLayoutDesc);

    struct BackgroundVertex
    {
        float x;
        float y;
        float u;
        float v;
    };

    VKW::VertexInputInfo backgroundVInfo;
    backgroundVInfo.binding_ = 0;
    backgroundVInfo.stride_ = sizeof(BackgroundVertex);
    backgroundVInfo.vertexAttributesCount_ = 2;
    backgroundVInfo.vertexAttributes_[0].location_ = 0;
    backgroundVInfo.vertexAttributes_[0].offset_ = 0;
    backgroundVInfo.vertexAttributes_[0].format_ = VK_FORMAT_R32G32_SFLOAT;
    backgroundVInfo.vertexAttributes_[1].location_ = 1;
    backgroundVInfo.vertexAttributes_[1].offset_ = sizeof(float) * 2;
    backgroundVInfo.vertexAttributes_[1].format_ = VK_FORMAT_R32G32_SFLOAT;

    Render::PipelineLayoutDesc backgroundLayoutDesc;
    backgroundLayoutDesc.staticMembersCount_ = 0;
    backgroundLayoutDesc.instancedMembersCount_ = 1;
    backgroundLayoutDesc.instancedMembers_[0] = backgroundSetLayoutKey;

    pipelineBackroundDesc.inputAssemblyInfo_ = &iaInfo;
    pipelineBackroundDesc.vertexInputInfo_ = &backgroundVInfo;
    pipelineBackroundDesc.viewportInfo_ = &vpInfo;
    pipelineBackroundDesc.depthStencilInfo_ = &dsInfo;
    pipelineBackroundDesc.layoutDesc_ = &backgroundLayoutDesc;
    pipelineBackroundDesc.dynamicStateFlags_ = VK_FLAGS_NONE;
    pipelineBackroundDesc.blendingState_ = VKW::PIPELINE_BLENDING_NONE;

    renderRoot_->DefineGraphicsPipeline(backgroundPipeKey, pipelineBackroundDesc);

    Render::MaterialTemplateDesc backgroundMaterialTemplateDesc;
    backgroundMaterialTemplateDesc.perPassDataCount_ = 1;
    backgroundMaterialTemplateDesc.perPassData_[0].passKey_ = passKey;
    backgroundMaterialTemplateDesc.perPassData_[0].pipelineKey_ = backgroundPipeKey;
    renderRoot_->DefineMaterialTemplate(backgroundMaterialTemplateKey, backgroundMaterialTemplateDesc);

    Render::MaterialDesc backgroundMaterialDesc;
    backgroundMaterialDesc.templateKey_ = backgroundMaterialTemplateKey;
    renderRoot_->DefineMaterial(backgroundMaterialKey, backgroundMaterialDesc);

    

    char const* backgroundVertexBufferName = "bcgvtx";
    std::uint32_t constexpr BACKGOUND_VERTICES_COUNT = 6;
    VKW::BufferViewDesc backgroundVertexBufferDesc;
    backgroundVertexBufferDesc.format_ = VK_FORMAT_UNDEFINED;
    backgroundVertexBufferDesc.size_ = sizeof(BackgroundVertex) * BACKGOUND_VERTICES_COUNT;
    backgroundVertexBufferDesc.usage_ = VKW::BufferUsage::VERTEX_INDEX;
    renderRoot_->DefineGlobalBuffer(backgroundVertexBufferName, backgroundVertexBufferDesc);

    BackgroundVertex backgroundVerticesData[BACKGOUND_VERTICES_COUNT];
    backgroundVerticesData[0].x = -1.0f;
    backgroundVerticesData[0].y = -1.0f;
    backgroundVerticesData[0].u = 0.0f;
    backgroundVerticesData[0].v = 0.0f;

    backgroundVerticesData[1].x = 1.0f;
    backgroundVerticesData[1].y = 1.0f;
    backgroundVerticesData[1].u = 1.0f;
    backgroundVerticesData[1].v = 1.0f;

    backgroundVerticesData[2].x = -1.0f;
    backgroundVerticesData[2].y = 1.0f;
    backgroundVerticesData[2].u = 0.0f;
    backgroundVerticesData[2].v = 1.0f;




    backgroundVerticesData[3].x = -1.0f;
    backgroundVerticesData[3].y = -1.0f;
    backgroundVerticesData[3].u = 0.0f;
    backgroundVerticesData[3].v = 0.0f;

    backgroundVerticesData[5].x = 1.0f;
    backgroundVerticesData[5].y = 1.0f;
    backgroundVerticesData[5].u = 1.0f;
    backgroundVerticesData[5].v = 1.0f;

    backgroundVerticesData[4].x = 1.0f;
    backgroundVerticesData[4].y = -1.0f;
    backgroundVerticesData[4].u = 1.0f;
    backgroundVerticesData[4].v = 0.0f;

    std::memcpy(mappedUploadBuffer, backgroundVerticesData, sizeof(BackgroundVertex) * BACKGOUND_VERTICES_COUNT);
    renderRoot_->FlushBuffer(uploadBufferKey, 0);
    renderRoot_->CopyStagingBufferToGPUBuffer(uploadBufferKey, backgroundVertexBufferName, 0);

    char const* backgroundTextureName = "bckground_texture";
    Data::Texture2D backgroundTextureData = ioManager_.ReadTexture2D("textures\\background.jpg", Data::TextureChannelVariations::TEXTURE_VARIATION_RGBA);
    std::memcpy(mappedUploadBuffer, backgroundTextureData.textureData_.data(), backgroundTextureData.textureData_.size());


    VKW::ImageViewDesc backgroundTextureDesc;
    backgroundTextureDesc.format_ = VK_FORMAT_R8G8B8A8_UNORM;
    backgroundTextureDesc.width_ = backgroundTextureData.width_;
    backgroundTextureDesc.height_ = backgroundTextureData.height_;
    backgroundTextureDesc.usage_ = VKW::ImageUsage::TEXTURE;
    renderRoot_->DefineGlobalImage(backgroundTextureName, backgroundTextureDesc);

    renderRoot_->CopyStagingBufferToGPUTexture(uploadBufferKey, backgroundTextureName, 0);
    VKW::ImageView* backgroundImageView = renderRoot_->FindGlobalImage(backgroundTextureName, 0);
    VKW::ImageResource* backgroundImageResource = renderRoot_->ResourceProxy()->GetResource(backgroundImageView->resource_);
    VkImageLayout backgroundImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    renderRoot_->ImageLayoutTransition(0, 1, &backgroundImageResource->handle_, &backgroundImageLayout);

    Render::RenderWorkItemDesc backgroundItemDesc;
    backgroundItemDesc.vertexBufferKey_ = backgroundVertexBufferName;
    backgroundItemDesc.indexBufferKey_ = "";
    backgroundItemDesc.vertexCount_ = BACKGOUND_VERTICES_COUNT;
    backgroundItemDesc.vertexBindOffset_ = 0;
    backgroundItemDesc.indexCount_ = 0;
    backgroundItemDesc.indexBindOffset_ = 0;

    backgroundItemDesc.setOwnerDescs_[0].members_[0].texture2D_.imageKey_ = backgroundTextureName;

    renderRoot_->RegisterMaterial(backgroundMaterialKey);
    Render::RenderWorkItemHandle backgroundRenderItemHandle = renderRoot_->ConstructRenderWorkItem(backgroundPipeKey, backgroundItemDesc);


}

void VulkanApplicationDelegate::InitImGui()
{
    if (imguiEnabled_)
        imguiHelper_->Init();
}



void VulkanApplicationDelegate::ImGuiUser(std::uint32_t context)
{
    if (imguiEnabled_) {
        IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!"); // Exceptionally add an extra assert here for people confused with initial dear imgui setup
        
        VKW::ImageView* colorBufferView = renderRoot_->FindGlobalImage(renderRoot_->GetDefaultSceneColorOutput(), 0);
        VKW::ImageResource* colorBufferResource = renderRoot_->ResourceProxy()->GetResource(colorBufferView->resource_);

        //ImGui::SetNextWindowContentWidth(100.0f);
        ImGui::SetNextWindowPos(ImVec2((float)colorBufferResource->width_ - 10.0f, 0.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));

        ImGuiWindowFlags frameDataWindowFlags = 
            ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoScrollbar | 
            ImGuiWindowFlags_NoCollapse | 
            ImGuiWindowFlags_NoInputs;
        static bool frameDataOpened = false;
        if (ImGui::Begin("Frame Stats", nullptr, frameDataWindowFlags))
        {
            ImGui::Text("DT: %f", prevFrameDelta_ * 0.001f);
            ImGui::Text("FPS: %f", 1.0f / prevFrameDelta_);
            ImGui::End();
        }
        

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f));
        
        ImGuiWindowFlags blurWindowFlags = 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Blur scale", nullptr, blurWindowFlags))
        {
            ImGui::SetNextItemWidth(100.0f);
            ImGui::SliderFloat("", &IMGUI_USER_BLUR_SCALE, 0.0f, 1.0f);
            ImGui::End();
        }


        //static bool p_open = true;
        //ImGui::ShowDemoWindow(&p_open);
    }
    
}

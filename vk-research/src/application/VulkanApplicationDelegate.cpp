#include "VulkanApplicationDelegate.hpp"

#include <vk_interface\Tools.hpp>
#include <renderer\Root.hpp>

#include <renderer\Material.hpp>
#include <transform\TansformComponent.hpp>

#include <renderer\CustomTempBlurPass.hpp>

#include <utility>
#include <cstdio>

#include <glm\gtc\quaternion.hpp>

#include <imgui\imgui.h>
#include <application\ImGuiUserData.hpp>

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

    for (std::uint32_t x = 0; x < 3; ++x) {
        for (std::uint32_t y = 0; y < 3; ++y) {
            customData_.transformComponents_[x * 3 + y]->scale_ = glm::vec3(5.0f);
            customData_.transformComponents_[x * 3 + y]->position_ = glm::vec3(-1.0f + x, -0.9f + y, -1.55f);
            customData_.transformComponents_[x * 3 + y]->orientation_.z = glm::degrees(3.14f);
            customData_.transformComponents_[x * 3 + y]->orientation_.y = glm::degrees(testcounter * 1.1f);
        }
    }
    
    // plane transform
    customData_.transformComponents_[CustomData::DRAGONS_COUNT]->scale_ = glm::vec3(0.5f);
    customData_.transformComponents_[CustomData::DRAGONS_COUNT]->position_ = glm::vec3(IMGUI_USER_PLANE_POS[0], IMGUI_USER_PLANE_POS[1], IMGUI_USER_PLANE_POS[2]);
    customData_.transformComponents_[CustomData::DRAGONS_COUNT]->orientation_ = glm::vec3(IMGUI_USER_PLANE_ROT[0], IMGUI_USER_PLANE_ROT[1], IMGUI_USER_PLANE_ROT[2]);


    //
    ////////////////////////////////////////////////////
    Transform::TransformSystemCameraData cameraData;
    cameraData.cameraPos = glm::vec3(IMGUI_USER_CAMERA_POS[0], IMGUI_USER_CAMERA_POS[1], IMGUI_USER_CAMERA_POS[2]);
    cameraData.cameraEuler = glm::vec3(IMGUI_USER_CAMERA_ROT[0], IMGUI_USER_CAMERA_ROT[1], IMGUI_USER_CAMERA_ROT[2]);
    cameraData.cameraFowDegrees = 60.0f;

    VKW::ImageView* colorBufferView = renderRoot_->FindGlobalImage(renderRoot_->GetDefaultSceneColorOutput(), 0);
    VKW::ImageResource* colorBufferResource = colorBufferView->resource_;
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
    char constexpr dragonPipeKey[] = "pipe0";
    char constexpr planePipeKey[] = "pipe1";
    char constexpr setLayoutKey[] = "set0";
    char constexpr planeSetLayoutKey[] = "set1";
    char constexpr materialTemplateKey[] = "templ0";
    char constexpr backgroundMaterialTemplateKey[] = "templ1";
    char constexpr materialKey[] = "mat0";
    char constexpr planeMaterialKey[] = "mat1";
    char constexpr depthBufferKey[] = "dpth0";

    char constexpr blurPass[] = "blur";

    Data::ModelMesh testMesh = ioManager_.ReadModelMesh("LFS\\dragon.model");

    if (testMesh.vertexData_.size() == 0 ||
        testMesh.indexData_.size() == 0) {
        std::cerr << "WARNING: LFS\\dragon.model failed to load. Consider pulling LFS folder." << std::endl;
    }

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
    VKW::ImageResource* colorBufferResource = colorBufferView->resource_;

    renderRoot_->DefineGlobalImage(depthBufferKey, VK_FORMAT_D16_UNORM, colorBufferResource->width_, colorBufferResource->height_, VKW::ImageUsage::DEPTH);

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

    renderRoot_->DefineGraphicsPipeline(dragonPipeKey, pipelineDesc);
    
    
    Render::MaterialTemplateDesc materialTemplateDesc;
    materialTemplateDesc.perPassDataCount_ = 1;
    materialTemplateDesc.perPassData_[0].passKey_ = passKey;
    materialTemplateDesc.perPassData_[0].pipelineKey_ = dragonPipeKey;
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

    itemDesc.setOwnerDescs_[0].members_[0].uniformBuffer_.size_ = 160;

    for (std::uint32_t i = 0; i < CustomData::DRAGONS_COUNT; ++i) {
        Render::RenderWorkItemHandle renderItemHandle = renderRoot_->ConstructRenderWorkItem(dragonPipeKey, itemDesc);

        Render::RenderWorkItem* item = renderRoot_->FindRenderWorkItem(dragonPipeKey, renderItemHandle);
        customData_.uniformProxies[i] = Render::UniformBufferWriterProxy(renderRoot_.get(), item, 0, 0);

        customData_.transformComponents_[i] = transformationSystem_.CreateTransformComponent(nullptr, &customData_.uniformProxies[i]);
    }
    


    renderRoot_->RegisterMaterial(materialKey);
    renderRoot_->PushPass(passKey);


    renderRoot_->DefineCustomBlurPass(blurPass, renderRoot_->GetDefaultSceneColorOutput(), &ioManager_);
    renderRoot_->PushPass(blurPass);



    // background pipeline, shaders, materials, renderworkitem
    Render::ShaderDesc planeVertexShaderDesc;
    planeVertexShaderDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_VERTEX;
    planeVertexShaderDesc.relativePath_ = "shader-src\\plane.vert.spv";

    Render::ShaderDesc planeFragmentShaderDesc;
    planeFragmentShaderDesc.type_ = VKW::ShaderModuleType::SHADER_MODULE_TYPE_FRAGMENT;
    planeFragmentShaderDesc.relativePath_ = "shader-src\\plane.frag.spv";

    renderRoot_->DefineShader("bvShader", planeVertexShaderDesc);
    renderRoot_->DefineShader("bfShader", planeFragmentShaderDesc);



    Render::GraphicsPipelineDesc pipelinePlaneDesc;

    pipelinePlaneDesc.renderPass_ = passKey;
    pipelinePlaneDesc.shaderStagesCount_ = 2;
    pipelinePlaneDesc.shaderStages_[0] = "bvShader";
    pipelinePlaneDesc.shaderStages_[1] = "bfShader";


    VKW::DescriptorSetLayoutDesc planeSetLayoutDesc;
    planeSetLayoutDesc.stage_ = VKW::DescriptorStage::RENDERING;
    planeSetLayoutDesc.membersCount_ = 3;
    planeSetLayoutDesc.membersDesc_[0].type_ = VKW::DESCRIPTOR_TYPE_TEXTURE;
    planeSetLayoutDesc.membersDesc_[0].binding_ = 0;
    planeSetLayoutDesc.membersDesc_[1].type_ = VKW::DESCRIPTOR_TYPE_TEXTURE;
    planeSetLayoutDesc.membersDesc_[1].binding_ = 1;
    planeSetLayoutDesc.membersDesc_[2].type_ = VKW::DESCRIPTOR_TYPE_TEXTURE;
    planeSetLayoutDesc.membersDesc_[2].binding_ = 2;
    renderRoot_->DefineSetLayout(planeSetLayoutKey, planeSetLayoutDesc);

    struct PlaneVertex
    {
        float pos[3];
        float norm[3];
        float tan[3];
        float bitan[3];
        float uv[2];
    };

    VKW::VertexInputInfo planeVertexInfo;
    planeVertexInfo.binding_ = 0;
    planeVertexInfo.stride_ = sizeof(PlaneVertex);
    planeVertexInfo.vertexAttributesCount_ = 5;
    planeVertexInfo.vertexAttributes_[0].location_ = 0;
    planeVertexInfo.vertexAttributes_[0].offset_ = 0;
    planeVertexInfo.vertexAttributes_[0].format_ = VK_FORMAT_R32G32B32_SFLOAT;
    planeVertexInfo.vertexAttributes_[1].location_ = 1;
    planeVertexInfo.vertexAttributes_[1].offset_ = offsetof(PlaneVertex, norm);
    planeVertexInfo.vertexAttributes_[1].format_ = VK_FORMAT_R32G32B32_SFLOAT;
    planeVertexInfo.vertexAttributes_[2].location_ = 2;
    planeVertexInfo.vertexAttributes_[2].offset_ = offsetof(PlaneVertex, tan);
    planeVertexInfo.vertexAttributes_[2].format_ = VK_FORMAT_R32G32B32_SFLOAT;
    planeVertexInfo.vertexAttributes_[3].location_ = 3;
    planeVertexInfo.vertexAttributes_[3].offset_ = offsetof(PlaneVertex, bitan);
    planeVertexInfo.vertexAttributes_[3].format_ = VK_FORMAT_R32G32B32_SFLOAT;
    planeVertexInfo.vertexAttributes_[4].location_ = 4;
    planeVertexInfo.vertexAttributes_[4].offset_ = offsetof(PlaneVertex, uv);
    planeVertexInfo.vertexAttributes_[4].format_ = VK_FORMAT_R32G32_SFLOAT;

    Render::PipelineLayoutDesc backgroundLayoutDesc;
    backgroundLayoutDesc.staticMembersCount_ = 0;
    backgroundLayoutDesc.instancedMembersCount_ = 2;
    backgroundLayoutDesc.instancedMembers_[0] = planeSetLayoutKey;
    backgroundLayoutDesc.instancedMembers_[1] = setLayoutKey;

    pipelinePlaneDesc.inputAssemblyInfo_ = &iaInfo;
    pipelinePlaneDesc.vertexInputInfo_ = &planeVertexInfo;
    pipelinePlaneDesc.viewportInfo_ = &vpInfo;
    pipelinePlaneDesc.depthStencilInfo_ = &dsInfo;
    pipelinePlaneDesc.layoutDesc_ = &backgroundLayoutDesc;
    pipelinePlaneDesc.dynamicStateFlags_ = VK_FLAGS_NONE;
    pipelinePlaneDesc.blendingState_ = VKW::PIPELINE_BLENDING_NONE;

    renderRoot_->DefineGraphicsPipeline(planePipeKey, pipelinePlaneDesc);

    Render::MaterialTemplateDesc planeMaterialTemplateDesc;
    planeMaterialTemplateDesc.perPassDataCount_ = 1;
    planeMaterialTemplateDesc.perPassData_[0].passKey_ = passKey;
    planeMaterialTemplateDesc.perPassData_[0].pipelineKey_ = planePipeKey;
    renderRoot_->DefineMaterialTemplate(backgroundMaterialTemplateKey, planeMaterialTemplateDesc);

    Render::MaterialDesc backgroundMaterialDesc;
    backgroundMaterialDesc.templateKey_ = backgroundMaterialTemplateKey;
    renderRoot_->DefineMaterial(planeMaterialKey, backgroundMaterialDesc);

    

    char const* planeVertexBufferName = "plnvtx";
    std::uint32_t constexpr PLANE_VERTICES_COUNT = 6;
    VKW::BufferViewDesc planeVertexBufferDesc;
    planeVertexBufferDesc.format_ = VK_FORMAT_UNDEFINED;
    planeVertexBufferDesc.size_ = sizeof(PlaneVertex) * PLANE_VERTICES_COUNT;
    planeVertexBufferDesc.usage_ = VKW::BufferUsage::VERTEX_INDEX;
    renderRoot_->DefineGlobalBuffer(planeVertexBufferName, planeVertexBufferDesc);

    PlaneVertex backgroundVerticesData[PLANE_VERTICES_COUNT];
    backgroundVerticesData[0].pos[0] = -1.0f;
    backgroundVerticesData[0].pos[1] = -1.0f;
    backgroundVerticesData[0].pos[2] = 0.0f;
    backgroundVerticesData[0].norm[0] = 0.0f;
    backgroundVerticesData[0].norm[1] = 0.0f;
    backgroundVerticesData[0].norm[2] = 1.0f;
    backgroundVerticesData[0].tan[0] = 1.0f;
    backgroundVerticesData[0].tan[1] = 0.0f;
    backgroundVerticesData[0].tan[2] = 0.0f;
    backgroundVerticesData[0].bitan[0] = 0.0f;
    backgroundVerticesData[0].bitan[1] = 1.0f;
    backgroundVerticesData[0].bitan[2] = 0.0f;
    backgroundVerticesData[0].uv[0]  = 0.0f;
    backgroundVerticesData[0].uv[1]  = 0.0f;

    backgroundVerticesData[1].pos[0] = 1.0f;
    backgroundVerticesData[1].pos[1] = 1.0f;
    backgroundVerticesData[1].pos[2] = 0.0f;
    backgroundVerticesData[1].norm[0] = 0.0f;
    backgroundVerticesData[1].norm[1] = 0.0f;
    backgroundVerticesData[1].norm[2] = 1.0f;
    backgroundVerticesData[1].tan[0] = 1.0f;
    backgroundVerticesData[1].tan[1] = 0.0f;
    backgroundVerticesData[1].tan[2] = 0.0f;
    backgroundVerticesData[1].bitan[0] = 0.0f;
    backgroundVerticesData[1].bitan[1] = 1.0f;
    backgroundVerticesData[1].bitan[2] = 0.0f;
    backgroundVerticesData[1].uv[0]  = 1.0f;
    backgroundVerticesData[1].uv[1]  = 1.0f;

    backgroundVerticesData[2].pos[0] = -1.0f;
    backgroundVerticesData[2].pos[1] = 1.0f;
    backgroundVerticesData[2].pos[2] = 0.0f;
    backgroundVerticesData[2].norm[0] = 0.0f;
    backgroundVerticesData[2].norm[1] = 0.0f;
    backgroundVerticesData[2].norm[2] = 1.0f;
    backgroundVerticesData[2].tan[0] = 1.0f;
    backgroundVerticesData[2].tan[1] = 0.0f;
    backgroundVerticesData[2].tan[2] = 0.0f;
    backgroundVerticesData[2].bitan[0] = 0.0f;
    backgroundVerticesData[2].bitan[1] = 1.0f;
    backgroundVerticesData[2].bitan[2] = 0.0f;
    backgroundVerticesData[2].uv[0]  = 0.0f;
    backgroundVerticesData[2].uv[1]  = 1.0f;


    backgroundVerticesData[3].pos[0] = -1.0f;
    backgroundVerticesData[3].pos[1] = -1.0f;
    backgroundVerticesData[3].pos[2] =  0.0f;
    backgroundVerticesData[3].norm[0] = 0.0f;
    backgroundVerticesData[3].norm[1] = 0.0f;
    backgroundVerticesData[3].norm[2] = 1.0f;
    backgroundVerticesData[3].tan[0] = 1.0f;
    backgroundVerticesData[3].tan[1] = 0.0f;
    backgroundVerticesData[3].tan[2] = 0.0f;
    backgroundVerticesData[3].bitan[0] = 0.0f;
    backgroundVerticesData[3].bitan[1] = 1.0f;
    backgroundVerticesData[3].bitan[2] = 0.0f;
    backgroundVerticesData[3].uv[0] = 0.0f;
    backgroundVerticesData[3].uv[1] = 0.0f;

    backgroundVerticesData[5].pos[0] = 1.0f;
    backgroundVerticesData[5].pos[1] = 1.0f;
    backgroundVerticesData[5].pos[2] = 0.0f;
    backgroundVerticesData[5].norm[0] = 0.0f;
    backgroundVerticesData[5].norm[1] = 0.0f;
    backgroundVerticesData[5].norm[2] = 1.0f;
    backgroundVerticesData[5].tan[0] = 1.0f;
    backgroundVerticesData[5].tan[1] = 0.0f;
    backgroundVerticesData[5].tan[2] = 0.0f;
    backgroundVerticesData[5].bitan[0] = 0.0f;
    backgroundVerticesData[5].bitan[1] = 1.0f;
    backgroundVerticesData[5].bitan[2] = 0.0f;
    backgroundVerticesData[5].uv[0] = 1.0f;
    backgroundVerticesData[5].uv[1] = 1.0f;

    backgroundVerticesData[4].pos[0] =  1.0f;
    backgroundVerticesData[4].pos[1] = -1.0f;
    backgroundVerticesData[4].pos[2] =  0.0f;
    backgroundVerticesData[4].norm[0] = 0.0f;
    backgroundVerticesData[4].norm[1] = 0.0f;
    backgroundVerticesData[4].norm[2] = 1.0f;
    backgroundVerticesData[4].tan[0] = 1.0f;
    backgroundVerticesData[4].tan[1] = 0.0f;
    backgroundVerticesData[4].tan[2] = 0.0f;
    backgroundVerticesData[4].bitan[0] = 0.0f;
    backgroundVerticesData[4].bitan[1] = 1.0f;
    backgroundVerticesData[4].bitan[2] = 0.0f;
    backgroundVerticesData[4].uv[0] = 1.0f;
    backgroundVerticesData[4].uv[1] = 0.0f;

    std::memcpy(mappedUploadBuffer, backgroundVerticesData, sizeof(PlaneVertex) * PLANE_VERTICES_COUNT);
    renderRoot_->FlushBuffer(uploadBufferKey, 0);
    renderRoot_->CopyStagingBufferToGPUBuffer(uploadBufferKey, planeVertexBufferName, 0);

    char const* planeTextureName = "bckground_texture";
    renderRoot_->CreateImageFromFile(ioManager_, uploadBufferKey, planeTextureName, "LFS\\Subway_Tiles_002_basecolor.jpg", VK_FORMAT_R8G8B8A8_UNORM, Data::TextureChannelVariations::TEXTURE_VARIATION_RGBA, VK_IMAGE_LAYOUT_GENERAL, VKW::ImageUsage::TEXTURE);

    char const* bumpTextureName = "bump_texture";
    renderRoot_->CreateImageFromFile(ioManager_, uploadBufferKey, bumpTextureName, "LFS\\Subway_Tiles_002_height.png", VK_FORMAT_R8_UNORM, Data::TextureChannelVariations::TEXTURE_VARIATION_GRAY, VK_IMAGE_LAYOUT_GENERAL, VKW::ImageUsage::TEXTURE);

    char const* normalTextureName = "normal_texture";
    renderRoot_->CreateImageFromFile(ioManager_, uploadBufferKey, normalTextureName, "LFS\\Subway_Tiles_002_normal.jpg", VK_FORMAT_R8G8B8A8_UNORM, Data::TextureChannelVariations::TEXTURE_VARIATION_RGBA, VK_IMAGE_LAYOUT_GENERAL, VKW::ImageUsage::TEXTURE);

    Render::RenderWorkItemDesc planeItemDesc;
    planeItemDesc.vertexBufferKey_ = planeVertexBufferName;
    planeItemDesc.indexBufferKey_ = "";
    planeItemDesc.vertexCount_ = PLANE_VERTICES_COUNT;
    planeItemDesc.vertexBindOffset_ = 0;
    planeItemDesc.indexCount_ = 0;
    planeItemDesc.indexBindOffset_ = 0;

    planeItemDesc.setOwnerDescs_[0].members_[0].texture2D_.imageKey_ = planeTextureName;
    planeItemDesc.setOwnerDescs_[0].members_[1].texture2D_.imageKey_ = bumpTextureName;
    planeItemDesc.setOwnerDescs_[0].members_[2].texture2D_.imageKey_ = normalTextureName;
    planeItemDesc.setOwnerDescs_[1].members_[0].uniformBuffer_.size_ = 160;

    renderRoot_->RegisterMaterial(planeMaterialKey);

    Render::RenderWorkItemHandle planeRenderItemHandle = renderRoot_->ConstructRenderWorkItem(planePipeKey, planeItemDesc);
    Render::RenderWorkItem* item = renderRoot_->FindRenderWorkItem(planePipeKey, planeRenderItemHandle);
    customData_.uniformProxies[CustomData::DRAGONS_COUNT] = Render::UniformBufferWriterProxy(renderRoot_.get(), item, 1, 0);
    customData_.transformComponents_[CustomData::DRAGONS_COUNT] = transformationSystem_.CreateTransformComponent(nullptr, &customData_.uniformProxies[CustomData::DRAGONS_COUNT]);

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
        VKW::ImageResource* colorBufferResource = colorBufferView->resource_;

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
        

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(0.0f, 100.0f));
        
        ImGuiWindowFlags blurWindowFlags = 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Blur scale", nullptr, blurWindowFlags))
        {
            static float TEST_FLOAT = 0.0f;
            ImGui::SetNextItemWidth(100.0f);
            ImGui::SliderFloat("Scale", &IMGUI_USER_BLUR_SCALE, 0.0f, 1.0f);
            
            ImGui::RadioButton("Fast blur", (int*)&IMGUI_USER_BLUR, (int)IMGUI_USER_BLUR_TYPE_FAST); ImGui::SameLine();
            ImGui::RadioButton("Full blur", (int*)&IMGUI_USER_BLUR, (int)IMGUI_USER_BLUR_TYPE_FULL); ImGui::SameLine();

            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImVec2(0.0f, 100.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(0.0f, 250.0f), ImGuiCond_Always);

        if (ImGui::Begin("Plane Settings", nullptr, ImGuiWindowFlags_NoResize))
        {
            if(ImGui::Button("Reset", ImVec2(50.0, 20.0f)))
            {
                ZeroMemory(IMGUI_USER_PLANE_POS, sizeof(IMGUI_USER_PLANE_POS));
                ZeroMemory(IMGUI_USER_PLANE_ROT, sizeof(IMGUI_USER_PLANE_ROT));
            }

            ImGui::Text("Plane position");
            
            char label_buf[16];
            char const x_char = 'x';

            for (std::uint32_t i = 0; i < 3; i++)
            {
                ImGui::SetNextItemWidth(200.0f);
                std::sprintf(label_buf, "pos_%c", x_char + i);
                ImGui::SliderFloat(label_buf, IMGUI_USER_PLANE_POS + i, -2.0f, 2.0f);
            }

            ImGui::NewLine();
            ImGui::Text("Plane euler rotation");

            for (std::uint32_t i = 0; i < 3; i++)
            {
                ImGui::SetNextItemWidth(200.0f);
                std::sprintf(label_buf, "rot_%c", x_char + i);
                ImGui::SliderFloat(label_buf, IMGUI_USER_PLANE_ROT + i, -80.0f, 80.0f);
            }

            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImVec2(0.0f, 350.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f), ImGuiCond_Always);

        if (ImGui::Begin("Camera Controls", nullptr, ImGuiWindowFlags_NoResize))
        {
            if (ImGui::Button("Reset", ImVec2(50.0, 20.0f)))
            {
                ZeroMemory(IMGUI_USER_CAMERA_POS, sizeof(IMGUI_USER_CAMERA_POS));
                ZeroMemory(IMGUI_USER_CAMERA_ROT, sizeof(IMGUI_USER_CAMERA_ROT));
            }

            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("up", ImGuiDir_Up))
                IMGUI_USER_CAMERA_POS[2] += 0.01f;

            ImGui::SameLine();

            if (ImGui::ArrowButton("left", ImGuiDir_Left))
                IMGUI_USER_CAMERA_POS[0] += 0.01f;

            ImGui::SameLine(0.0f, 20.0f);

            if(ImGui::ArrowButton("upr", ImGuiDir_Up))
                IMGUI_USER_CAMERA_ROT[0] += 0.05f;

            ImGui::SameLine();

            if(ImGui::ArrowButton("leftr", ImGuiDir_Left))
                IMGUI_USER_CAMERA_ROT[1] -= 0.05f;

            // new line

            if (ImGui::ArrowButton("down", ImGuiDir_Down))
                IMGUI_USER_CAMERA_POS[2] -= 0.01f;

            ImGui::SameLine();

            if (ImGui::ArrowButton("right", ImGuiDir_Right))
                IMGUI_USER_CAMERA_POS[0] -= 0.01f;

            ImGui::SameLine(0.0f, 20.0f);

            if (ImGui::ArrowButton("downr", ImGuiDir_Down))
                IMGUI_USER_CAMERA_ROT[0] -= 0.05f;

            ImGui::SameLine();

            if (ImGui::ArrowButton("rightr", ImGuiDir_Right))
                IMGUI_USER_CAMERA_ROT[1] += 0.05f;

            ImGui::PopButtonRepeat();

            ImGui::Text("Camera pos: %.2f, %.2f, %.2f", IMGUI_USER_CAMERA_POS[0], IMGUI_USER_CAMERA_POS[1], IMGUI_USER_CAMERA_POS[2]);
            ImGui::Text("Camera rot: %.2f, %.2f, %.2f", IMGUI_USER_CAMERA_ROT[0], IMGUI_USER_CAMERA_ROT[1], IMGUI_USER_CAMERA_ROT[2]);

            ImGui::End();
        }


        //static bool p_open = true;
        //ImGui::ShowDemoWindow(&p_open);
    }
    
}

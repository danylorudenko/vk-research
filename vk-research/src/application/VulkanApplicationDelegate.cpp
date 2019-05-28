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
    FakeParseRendererResources();

    if (imguiEnabled_)
        InitImGui();
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
    customData_.transformComponent_->position_ = glm::vec3(-0.5f, 0.0f, -1.0f);
    //customData_.transformComponent_->orientation_.z = glm::degrees(testcounter);
    customData_.transformComponent_->orientation_.z = glm::degrees(3.14f);
    customData_.transformComponent_->orientation_.y = glm::degrees(testcounter * 1.1f);
    

    //
    ////////////////////////////////////////////////////
    transformationSystem_.Update(context, glm::vec3(0.0f), glm::vec3(0.0f), 60.0f);

    VKW::WorkerFrameCommandReciever commandReciever = renderRoot_->BeginRenderGraph(presentationContext);
    renderRoot_->IterateRenderGraph(presentationContext, commandReciever);

    TestImGui(context);


    if (imguiEnabled_) {
        imguiHelper_->EndFrame(context);
        imguiHelper_->Render(context, commandReciever);
    }

    renderRoot_->EndRenderGraph(presentationContext);
}

void VulkanApplicationDelegate::shutdown()
{
    
}

void VulkanApplicationDelegate::FakeParseRendererResources()
{
    //VKW::DescriptorSetLayoutDesc setLayoutDesc;
    //setLayoutDesc.membersCount_ = 3;
    //
    //setLayoutDesc.membersDesc_[0].type_ = VKW::DESCRIPTOR_TYPE_TEXTURE;
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
    passDesc.colorAttachments_[0].resourceKey_ = Render::Root::SWAPCHAIN_IMAGE_KEY; // we need swapchain reference here
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
    itemDesc.indexBindOffset_ = 0;

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

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void VulkanApplicationDelegate::TestImGui(std::uint32_t context)
{
    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!"); // Exceptionally add an extra assert here for people confused with initial dear imgui setup

                                                                                                           // Examples Apps (accessible from the "Examples" menu)
    static bool show_app_documents = false;
    static bool show_app_main_menu_bar = false;
    static bool show_app_console = false;
    static bool show_app_log = false;
    static bool show_app_layout = false;
    static bool show_app_property_editor = false;
    static bool show_app_long_text = false;
    static bool show_app_auto_resize = false;
    static bool show_app_constrained_resize = false;
    static bool show_app_simple_overlay = false;
    static bool show_app_window_titles = false;
    static bool show_app_custom_rendering = false;

    //if (show_app_documents)           ShowExampleAppDocuments(&show_app_documents);
    //if (show_app_main_menu_bar)       ShowExampleAppMainMenuBar();
    //if (show_app_console)             ShowExampleAppConsole(&show_app_console);
    //if (show_app_log)                 ShowExampleAppLog(&show_app_log);
    //if (show_app_layout)              ShowExampleAppLayout(&show_app_layout);
    //if (show_app_property_editor)     ShowExampleAppPropertyEditor(&show_app_property_editor);
    //if (show_app_long_text)           ShowExampleAppLongText(&show_app_long_text);
    //if (show_app_auto_resize)         ShowExampleAppAutoResize(&show_app_auto_resize);
    //if (show_app_constrained_resize)  ShowExampleAppConstrainedResize(&show_app_constrained_resize);
    //if (show_app_simple_overlay)      ShowExampleAppSimpleOverlay(&show_app_simple_overlay);
    //if (show_app_window_titles)       ShowExampleAppWindowTitles(&show_app_window_titles);
    //if (show_app_custom_rendering)    ShowExampleAppCustomRendering(&show_app_custom_rendering);

    // Dear ImGui Apps (accessible from the "Help" menu)
    static bool show_app_metrics = false;
    static bool show_app_style_editor = false;
    static bool show_app_about = false;

    if (show_app_metrics) { ImGui::ShowMetricsWindow(&show_app_metrics); }
    //if (show_app_style_editor) { ImGui::Begin("Style Editor", &show_app_style_editor); ImGui::ShowStyleEditor(); ImGui::End(); }
    //if (show_app_about) { ImGui::ShowAboutWindow(&show_app_about); }

    // Demonstrate the various window flags. Typically you would just use the default!
    static bool no_titlebar = false;
    static bool no_scrollbar = false;
    static bool no_menu = false;
    static bool no_move = false;
    static bool no_resize = false;
    static bool no_collapse = false;
    static bool no_close = false;
    static bool no_nav = false;
    static bool no_background = false;
    static bool no_bring_to_front = false;

    ImGuiWindowFlags window_flags = 0;
    if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
    if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
    if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
    if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
    if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
    if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
    if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    //if (no_close)           p_open = NULL; // Don't pass our bool* to Begin

                                           // We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only do it to make the Demo applications a little more welcoming.
    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

    static bool g_open = true;
    bool* p_open = &g_open;
    // Main body of the Demo window starts here.
    if (!ImGui::Begin("ImGui Demo", p_open, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    // Most "big" widgets share a common width settings by default.
    //ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);    // Use 2/3 of the space for widgets and 1/3 for labels (default)
    ImGui::PushItemWidth(ImGui::GetFontSize() * -12);           // Use fixed width for labels (by passing a negative value), the rest goes to widgets. We choose a width proportional to our font size.

                                                                // Menu Bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            //ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Examples"))
        {
            ImGui::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);
            ImGui::MenuItem("Console", NULL, &show_app_console);
            ImGui::MenuItem("Log", NULL, &show_app_log);
            ImGui::MenuItem("Simple layout", NULL, &show_app_layout);
            ImGui::MenuItem("Property editor", NULL, &show_app_property_editor);
            ImGui::MenuItem("Long text display", NULL, &show_app_long_text);
            ImGui::MenuItem("Auto-resizing window", NULL, &show_app_auto_resize);
            ImGui::MenuItem("Constrained-resizing window", NULL, &show_app_constrained_resize);
            ImGui::MenuItem("Simple overlay", NULL, &show_app_simple_overlay);
            ImGui::MenuItem("Manipulating window titles", NULL, &show_app_window_titles);
            ImGui::MenuItem("Custom rendering", NULL, &show_app_custom_rendering);
            ImGui::MenuItem("Documents", NULL, &show_app_documents);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::MenuItem("Metrics", NULL, &show_app_metrics);
            ImGui::MenuItem("Style Editor", NULL, &show_app_style_editor);
            ImGui::MenuItem("About Dear ImGui", NULL, &show_app_about);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::Text("dear imgui says hello. (%s)", IMGUI_VERSION);
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Help"))
    {
        ImGui::Text("PROGRAMMER GUIDE:");
        ImGui::BulletText("Please see the ShowDemoWindow() code in imgui_demo.cpp. <- you are here!");
        ImGui::BulletText("Please see the comments in imgui.cpp.");
        ImGui::BulletText("Please see the examples/ in application.");
        ImGui::BulletText("Enable 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.");
        ImGui::BulletText("Enable 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.");
        ImGui::Separator();

        ImGui::Text("USER GUIDE:");
        //ImGui::ShowUserGuide();
    }

    if (ImGui::CollapsingHeader("Configuration"))
    {
        ImGuiIO& io = ImGui::GetIO();

        if (ImGui::TreeNode("Configuration##2"))
        {
            ImGui::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard", (unsigned int *)&io.ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);
            ImGui::CheckboxFlags("io.ConfigFlags: NavEnableGamepad", (unsigned int *)&io.ConfigFlags, ImGuiConfigFlags_NavEnableGamepad);
            ImGui::SameLine(); HelpMarker("Required back-end to feed in gamepad inputs in io.NavInputs[] and set io.BackendFlags |= ImGuiBackendFlags_HasGamepad.\n\nRead instructions in imgui.cpp for details.");
            ImGui::CheckboxFlags("io.ConfigFlags: NavEnableSetMousePos", (unsigned int *)&io.ConfigFlags, ImGuiConfigFlags_NavEnableSetMousePos);
            ImGui::SameLine(); HelpMarker("Instruct navigation to move the mouse cursor. See comment for ImGuiConfigFlags_NavEnableSetMousePos.");
            ImGui::CheckboxFlags("io.ConfigFlags: NoMouse", (unsigned int *)&io.ConfigFlags, ImGuiConfigFlags_NoMouse);
            if (io.ConfigFlags & ImGuiConfigFlags_NoMouse) // Create a way to restore this flag otherwise we could be stuck completely!
            {
                if (fmodf((float)ImGui::GetTime(), 0.40f) < 0.20f)
                {
                    ImGui::SameLine();
                    ImGui::Text("<<PRESS SPACE TO DISABLE>>");
                }
                if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space)))
                    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            }
            ImGui::CheckboxFlags("io.ConfigFlags: NoMouseCursorChange", (unsigned int *)&io.ConfigFlags, ImGuiConfigFlags_NoMouseCursorChange);
            ImGui::SameLine(); HelpMarker("Instruct back-end to not alter mouse cursor shape and visibility.");
            ImGui::Checkbox("io.ConfigInputTextCursorBlink", &io.ConfigInputTextCursorBlink);
            ImGui::SameLine(); HelpMarker("Set to false to disable blinking cursor, for users who consider it distracting");
            ImGui::Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges);
            ImGui::SameLine(); HelpMarker("Enable resizing of windows from their edges and from the lower-left corner.\nThis requires (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors) because it needs mouse cursor feedback.");
            ImGui::Checkbox("io.ConfigWindowsMoveFromTitleBarOnly", &io.ConfigWindowsMoveFromTitleBarOnly);
            ImGui::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
            ImGui::SameLine(); HelpMarker("Instruct Dear ImGui to render a mouse cursor for you. Note that a mouse cursor rendered via your application GPU rendering path will feel more laggy than hardware cursor, but will be more in sync with your other visuals.\n\nSome desktop applications may use both kinds of cursors (e.g. enable software cursor only when resizing/dragging something).");
            ImGui::TreePop();
            ImGui::Separator();
        }

        if (ImGui::TreeNode("Backend Flags"))
        {
            HelpMarker("Those flags are set by the back-ends (imgui_impl_xxx files) to specify their capabilities.");
            ImGuiBackendFlags backend_flags = io.BackendFlags; // Make a local copy to avoid modifying actual back-end flags.
            ImGui::CheckboxFlags("io.BackendFlags: HasGamepad", (unsigned int *)&backend_flags, ImGuiBackendFlags_HasGamepad);
            ImGui::CheckboxFlags("io.BackendFlags: HasMouseCursors", (unsigned int *)&backend_flags, ImGuiBackendFlags_HasMouseCursors);
            ImGui::CheckboxFlags("io.BackendFlags: HasSetMousePos", (unsigned int *)&backend_flags, ImGuiBackendFlags_HasSetMousePos);
            ImGui::TreePop();
            ImGui::Separator();
        }

        if (ImGui::TreeNode("Style"))
        {
            //ImGui::ShowStyleEditor();
            ImGui::TreePop();
            ImGui::Separator();
        }

        if (ImGui::TreeNode("Capture/Logging"))
        {
            ImGui::TextWrapped("The logging API redirects all text output so you can easily capture the content of a window or a block. Tree nodes can be automatically expanded.");
            HelpMarker("Try opening any of the contents below in this window and then click one of the \"Log To\" button.");
            ImGui::LogButtons();
            ImGui::TextWrapped("You can also call ImGui::LogText() to output directly to the log without a visual output.");
            if (ImGui::Button("Copy \"Hello, world!\" to clipboard"))
            {
                ImGui::LogToClipboard();
                ImGui::LogText("Hello, world!");
                ImGui::LogFinish();
            }
            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Window options"))
    {
        ImGui::Checkbox("No titlebar", &no_titlebar); ImGui::SameLine(150);
        ImGui::Checkbox("No scrollbar", &no_scrollbar); ImGui::SameLine(300);
        ImGui::Checkbox("No menu", &no_menu);
        ImGui::Checkbox("No move", &no_move); ImGui::SameLine(150);
        ImGui::Checkbox("No resize", &no_resize); ImGui::SameLine(300);
        ImGui::Checkbox("No collapse", &no_collapse);
        ImGui::Checkbox("No close", &no_close); ImGui::SameLine(150);
        ImGui::Checkbox("No nav", &no_nav); ImGui::SameLine(300);
        ImGui::Checkbox("No background", &no_background);
        ImGui::Checkbox("No bring to front", &no_bring_to_front);
    }

    // All demo contents
    //ShowDemoWindowWidgets();
    //ShowDemoWindowLayout();
    //ShowDemoWindowPopups();
    //ShowDemoWindowColumns();
    //ShowDemoWindowMisc();

    // End of ShowDemoWindow()
    ImGui::End();
}

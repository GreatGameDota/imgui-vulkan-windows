#include "Application.h"

#include "gui/Application.h"

static std::default_random_engine r_eng;
static std::uniform_real_distribution<> dis(-1.5f, 1.5f); // range [0, 1)

int Application::Init()
{
    running = true;
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Create window with Vulkan context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // glfwWindowHint(GLFW_TITLEBAR, false);
    // glfwWindowHint(GLFW_DECORATED, false);

    // glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    window = glfwCreateWindow(900, 600, "Vulkan", nullptr, nullptr);
    // glfwShowWindow(window);

    if (!glfwVulkanSupported())
    {
        printf("GLFW: Vulkan Not Supported\n");
        return 1;
    }

    // // Create Window Surface
    // err = glfwCreateWindowSurface(g_Instance, window, g_Allocator, &surface);
    // check_vk_result(err);

    ImVector<const char *> extensions;
    uint32_t extensions_count = 0;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
    for (uint32_t i = 0; i < extensions_count; i++)
        extensions.push_back(glfw_extensions[i]);
    SetupVulkan(extensions);

    // Create Framebuffers
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, w, h);

    r_eng.seed((uint32_t)time(NULL));

    vulkanWindow = new VulkanWindow(g_Instance,
                                    g_PhysicalDevice,
                                    g_Device,
                                    g_Queue,
                                    g_PresentQueue,
                                    g_QueueFamily,
                                    g_PresentQueueFamily);

    vulkanWindow->addVertices(vulkanWindow->verticesAll);
    // for (int i = 0; i < 100000; i++)
    // {
    //     vulkanWindow->addVertex({{dis(r_eng), dis(r_eng), dis(r_eng)}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}});
    //     vulkanWindow->addVertex({{dis(r_eng), dis(r_eng), dis(r_eng)}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}});
    //     vulkanWindow->addVertex({{dis(r_eng), dis(r_eng), dis(r_eng)}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}});
    // }
    vulkanWindow->updateIndexBuffer();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    io = &ImGui::GetIO();
    // (void)io;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(10.0f, 10.0f); // Padding around main window
    style.FramePadding = ImVec2(8.0f, 6.0f);    // Inner padding of items
    // style.ItemSpacing = ImVec2(6.0f, 6.0f);      // Padding between items
    // style.WindowTitleAlign = ImVec2(0.5f, 0.5f); // Center window titles

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.RenderPass = wd->RenderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = g_Allocator;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);

    // Load Fonts
    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false;
    ImFont *robotoFont = io->Fonts->AddFontFromMemoryTTF((void *)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
    s_Fonts["Default"] = robotoFont;
    s_Fonts["Bold"] = io->Fonts->AddFontFromMemoryTTF((void *)g_RobotoBold, sizeof(g_RobotoBold), 20.0f, &fontConfig);
    s_Fonts["Italic"] = io->Fonts->AddFontFromMemoryTTF((void *)g_RobotoItalic, sizeof(g_RobotoItalic), 20.0f, &fontConfig);
    io->FontDefault = robotoFont;

    s_Fonts["ConsoleRegular"] = io->Fonts->AddFontFromMemoryTTF((void *)FiraCode_Regular_ttf, sizeof(FiraCode_Regular_ttf), 20.0f, &fontConfig);

    return 0;
}

void Application::Run()
{
    // Our state
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window) && !glfwWindowShouldClose(vulkanWindow->m_Window2))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Resize swap chain?
        int fb_width, fb_height;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        if (fb_width > 0 && fb_height > 0 && (g_SwapChainRebuild || g_MainWindowData.Width != fb_width || g_MainWindowData.Height != fb_height))
        {
            ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
            ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData, g_QueueFamily, g_Allocator, fb_width, fb_height, g_MinImageCount);
            g_MainWindowData.FrameIndex = 0;
            g_SwapChainRebuild = false;
        }
        // if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        // {
        //     ImGui_ImplGlfw_Sleep(10);
        //     continue;
        // }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create the docking environment
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                       ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                       ImGuiWindowFlags_NoBackground;

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("InvisibleWindow", nullptr, windowFlags);
        ImGui::PopStyleVar(3);

        ImGuiID dockSpaceId = ImGui::GetID("InvisibleWindowDockSpace");

        ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGui::Begin("Hello, world!");                     // Create a window called "Hello, world!" and append into it.
            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state

            ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);

            static float f1 = -0.5f;
            ImGui::SliderFloat("slider float", &f1, -2.0f, 2.0f, "ratio = %.3f");
            // if (vulkanWindow->vertices[0].pos[0] != f1)
            // {
            //     vulkanWindow->vertices[0].pos[0] = f1;
            //     vulkanWindow->vertices[3].pos[0] = f1;
            //     // vulkanWindow->vertices[4].pos[0] = f1;
            //     // vulkanWindow->vertices[5].pos[0] = f1;
            //     vulkanWindow->updateIndexBuffer();
            // }
            static float f2 = 0.0f;
            ImGui::SliderFloat("slider float2", &f2, -2.0f, 2.0f, "ratio = %.3f");
            // if (vulkanWindow->vertices[0].pos[2] != f2)
            // {
            //     vulkanWindow->vertices[0].pos[2] = f2;
            //     vulkanWindow->vertices[1].pos[2] = f2;
            //     vulkanWindow->vertices[2].pos[2] = f2;
            //     vulkanWindow->vertices[3].pos[2] = f2;
            //     // vulkanWindow->vertices[4].pos[2] = f2;
            //     // vulkanWindow->vertices[5].pos[2] = f2;
            //     vulkanWindow->updateIndexBuffer();
            // }
            // if (f2 < -1)
            // {
            //     vulkanWindow->numVerts = 0;
            //     vulkanWindow->numInts = 0;
            //     for (int i = 0; i < 100000; i++)
            //     {
            //         vulkanWindow->addVertex({{dis(r_eng), dis(r_eng), dis(r_eng)}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}});
            //         vulkanWindow->addVertex({{dis(r_eng), dis(r_eng), dis(r_eng)}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}});
            //         vulkanWindow->addVertex({{dis(r_eng), dis(r_eng), dis(r_eng)}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}});
            //     }
            //     vulkanWindow->updateIndexBuffer();
            // }

            ImGui::End();
        }

        // for (const auto &widget : widgets)
        //     widget->Render();
        // vulkanWindow->verticesAll.push_back({{1.0f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}});

        // Rendering
        ImGui::Render();
        ImDrawData *main_draw_data = ImGui::GetDrawData();
        const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
        wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
        wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
        wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
        wd->ClearValue.color.float32[3] = clear_color.w;
        if (!main_is_minimized)
            FrameRender(wd, main_draw_data);

        // Update and Render additional Platform Windows
        if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        if (!main_is_minimized)
            FramePresent(wd);

        if (glfwGetWindowAttrib(vulkanWindow->m_Window2, GLFW_ICONIFIED) != 0)
        {
            // ImGui_ImplGlfw_Sleep(10);
            continue;
        }
        vulkanWindow->drawFrame2();
    }
    running = false;
}

void Application::Destroy()
{
    running = false;
    vkDeviceWaitIdle(g_Device);
    // Cleanup
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();

    CleanupVulkanWindow();
    CleanupVulkan();

    glfwDestroyWindow(window);
    glfwTerminate();
}

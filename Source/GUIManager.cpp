#include "Headers/GUIManager.h"

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

void GUIManager::createImGuiDescriptorPool()
{
    VkDescriptorPoolSize imGui_pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2},
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(imGui_pool_sizes);
    pool_info.pPoolSizes = imGui_pool_sizes;
    VkResult err = vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_imGuiDescriptorPool);
    check_vk_result(err);
}

void GUIManager::createImGuiFramebuffer(std::vector<VkImageView>& swapChainImageViews)
{
    m_imGuiFrameBuffers.resize(swapChainImageViews.size());

    VkImageView attachment[1];
    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = m_imGuiRenderPass;
    info.attachmentCount = 1;
    info.pAttachments = attachment;
    info.width = swapChainExtent.width;
    info.height = swapChainExtent.height;
    info.layers = 1;

    for (uint32_t i = 0; i < swapChainImageViews.size(); i++)
    {
        attachment[0] = swapChainImageViews[i];
        if (vkCreateFramebuffer(m_device, &info, nullptr, &m_imGuiFrameBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void GUIManager::createImGuiRenderPass(VkFormat swapChainImageFormat)
{
    VkAttachmentDescription guiAttachment{};
    guiAttachment.format = swapChainImageFormat;
    guiAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    guiAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    guiAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    guiAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    guiAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    guiAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    guiAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo guiRenderPassInfo = {};
    guiRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    guiRenderPassInfo.attachmentCount = 1;
    guiRenderPassInfo.pAttachments = &guiAttachment;
    guiRenderPassInfo.subpassCount = 1;
    guiRenderPassInfo.pSubpasses = &subpass;
    guiRenderPassInfo.dependencyCount = 1;
    guiRenderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device, &guiRenderPassInfo, nullptr, &m_imGuiRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

}

void GUIManager::createImGuiCommandBuffers()
{
    m_imGuiCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_imGuiCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_imGuiCommandBuffers.size();

    if (vkAllocateCommandBuffers(m_device, &allocInfo, m_imGuiCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate imgui command buffers!");
    }
}

void GUIManager::initImGUIAttribute(const VkDevice& in_device, VkExtent2D& in_swapChainExtent,VkFormat& swapChainImageFormat, std::vector<VkImageView>& swapChainImageViews, uint32_t width, uint32_t height)
{
    m_device = in_device;
    swapChainExtent = in_swapChainExtent;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &(ImGui::GetIO());

    (void)io;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    //Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();

    ImFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 1;
    config.GlyphExtraSpacing.x = 1.0f;

    // io.Fonts->AddFontDefault();
    io->Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 24.0f, &config);
}

void GUIManager::destroy()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(m_device, m_imGuiDescriptorPool, nullptr);
    vkDestroyCommandPool(m_device, m_imGuiCommandPool, nullptr);
    vkDestroyRenderPass(m_device, m_imGuiRenderPass, nullptr);
}

void GUIManager::createGUIFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    //Show a simple window that we create for ourselves
    static float f = 0.0f;
    static int counter = 0;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Load Model")) 
            {
                std::cout << "Trigger Load Model" << std::endl;
                openFile(loadModelfilePath);
                //std::string loadTexturePath;
                if (loadModelfilePath != "")
                {
                    mainScene->loadModel(loadModelfilePath);
                }
            }
            if (ImGui::MenuItem("Load texture"))
            {
                std::cout << "Trigger Load Texture" << std::endl;
            }
            if (ImGui::MenuItem("Save Screen Shotcut"))
            {
                std::cout << "Trigger Screen Shotcut" << std::endl;
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::Text(" Current : %.1f  FPS ",  io->Framerate);
    ImGui::SameLine();
    ImGui::Render();
    m_drawData = ImGui::GetDrawData();
}

void GUIManager::drawUI(uint32_t currentFrame,uint32_t imageIndex)
{
    vkResetCommandBuffer(m_imGuiCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    VkCommandBufferBeginInfo imGui_beginInfo{};
    imGui_beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_imGuiCommandBuffers[currentFrame], &imGui_beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin gui command buffer!");
    }

    VkRenderPassBeginInfo imGui_renderPassInfo{};
    imGui_renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    imGui_renderPassInfo.renderPass = m_imGuiRenderPass;
    imGui_renderPassInfo.framebuffer = m_imGuiFrameBuffers[imageIndex];
    imGui_renderPassInfo.renderArea.offset = { 0, 0 };
    imGui_renderPassInfo.renderArea.extent = swapChainExtent;
    VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    imGui_renderPassInfo.clearValueCount = 1;
    imGui_renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(m_imGuiCommandBuffers[currentFrame], &imGui_renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(m_drawData, m_imGuiCommandBuffers[currentFrame]);

    vkCmdEndRenderPass(m_imGuiCommandBuffers[currentFrame]);
    vkEndCommandBuffer(m_imGuiCommandBuffers[currentFrame]);
}

bool GUIManager::openFile(std::string& selected_file_path)
{
    std::string sFilePath;
    //  CREATE FILE OBJECT INSTANCE
    HRESULT f_sysHr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(f_sysHr))
        return FALSE;

    //Create FileOpenDialogue OBJECT
    IFileOpenDialog* f_FileSystem;
    f_sysHr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&f_FileSystem));
    if (FAILED(f_sysHr)) {
        CoUninitialize();
        return FALSE;
    }

    //  SHOW OPEN FILE DIALOG WINDOW
    f_sysHr = f_FileSystem->Show(NULL);
    if (FAILED(f_sysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  RETRIEVE FILE NAME FROM THE SELECTED ITEM
    IShellItem* f_Files;
    f_sysHr = f_FileSystem->GetResult(&f_Files);
    if (FAILED(f_sysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  STORE AND CONVERT THE FILE NAME
    PWSTR f_Path;
    f_sysHr = f_Files->GetDisplayName(SIGDN_FILESYSPATH, &f_Path);
    if (FAILED(f_sysHr)) {
        f_Files->Release();
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }
    //  FORMAT AND STORE THE FILE PATH
    std::wstring path(f_Path);
    std::string c(path.begin(), path.end());
    selected_file_path = c;

    //  FORMAT STRING FOR EXECUTABLE NAME
    //const size_t slash = sFilePath.find_last_of("/\\");
    //selected_file = sFilePath.substr(slash + 1);

    //  SUCCESS, CLEAN UP
    CoTaskMemFree(f_Path);
    f_Files->Release();
    f_FileSystem->Release();
    CoUninitialize();
    return TRUE;
}
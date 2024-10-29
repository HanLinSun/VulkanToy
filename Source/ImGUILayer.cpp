#include <ImGUILayer.h>
#include <Application.h>
#include <Vulkan/Initializer.hpp>

namespace Renderer
{
	ImGuiLayer::ImGuiLayer() :Layer("ImGui Layer")
	{

	}

	void ImGuiLayer::OnUpdate()
	{
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        //Show a simple window that we create for ourselves
        static float f = 0.0f;
        static int counter = 0;


        ImGui::Text(" Current : %.1f  FPS ", io->Framerate);
        ImGui::SameLine();
        ImGui::Render();
        m_drawData = ImGui::GetDrawData();
	}

    void ImGuiLayer::OnDetach()
    {
    }

    VkDescriptorPool ImGuiLayer::GetDescriptorPool() const
    {
        return m_descriptorPool;
    }

    VkCommandPool ImGuiLayer::GetCommandPool() const
    {
        return m_commandPool;
    }

    void ImGuiLayer::InitVulkanResources()
    {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* fontData;
        int texWidth, texHeight;
        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
        VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

        
        // Create target image for copy
        Tools::CreateImage(m_device, texWidth, texHeight, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_fontImage, m_fontMemory);

        // Image view
        VkImageViewCreateInfo viewInfo = VulkanInitializer::imageViewCreateInfo();
        viewInfo.image = m_fontImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;
        check_vk_result(vkCreateImageView(m_device->GetVkDevice(), &viewInfo, nullptr, &m_fontView));

        // Staging buffers for font data upload
        Buffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        check_vk_result(BufferUtils::CreateBuffer(m_device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, uploadSize));

        stagingBuffer.Map();
        memcpy(stagingBuffer.mapped, fontData, uploadSize);
        stagingBuffer.Unmap();

    }

    void ImGuiLayer::OnEvent(Event& e)
    {

    }
    void ImGuiLayer::DrawUI(uint32_t currentFrame, uint32_t imageIndex)
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
        imGui_renderPassInfo.renderArea.extent = m_swapChainExtent;
        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        imGui_renderPassInfo.clearValueCount = 1;
        imGui_renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(m_imGuiCommandBuffers[currentFrame], &imGui_renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        ImGui_ImplVulkan_RenderDrawData(m_drawData, m_imGuiCommandBuffers[currentFrame]);

        vkCmdEndRenderPass(m_imGuiCommandBuffers[currentFrame]);
        vkEndCommandBuffer(m_imGuiCommandBuffers[currentFrame]);
    }

    void ImGuiLayer::CreateImGuiDescriptorPool()
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
        VkResult err = vkCreateDescriptorPool(m_device->GetVkDevice(), &pool_info, nullptr, &m_descriptorPool);
        check_vk_result(err);
    }

    void ImGuiLayer::CreateImGuiRenderPass(VkFormat swapChainImageFormat)
    {
        VkAttachmentDescription guiAttachment{};
        guiAttachment.format = swapChainImageFormat;
        guiAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        guiAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        guiAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        guiAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        guiAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        guiAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        guiAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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

        if (vkCreateRenderPass(m_device->GetVkDevice(), &guiRenderPassInfo, nullptr, &m_imGuiRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void ImGuiLayer::CreateImGuiCommandBuffers()
    {
        m_imGuiCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)m_imGuiCommandBuffers.size();

        if (vkAllocateCommandBuffers(m_device->GetVkDevice(), &allocInfo, m_imGuiCommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate imgui command buffers!");
        }
    }

    void ImGuiLayer::CreateImGuiFramebuffer(std::vector<VkImageView>& swapChainImageViews)
    {
        m_imGuiFrameBuffers.resize(swapChainImageViews.size());
        VkImageView attachment[1];
        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = m_imGuiRenderPass;
        info.attachmentCount = 1;
        info.pAttachments = attachment;
        info.width = m_swapChainExtent.width;
        info.height = m_swapChainExtent.height;
        info.layers = 1;

        for (uint32_t i = 0; i < swapChainImageViews.size(); i++)
        {
            attachment[0] = swapChainImageViews[i];
            if (vkCreateFramebuffer(m_device->GetVkDevice(), &info, nullptr, &m_imGuiFrameBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void ImGuiLayer::InitImGUIAttribute(Device*  device, VkExtent2D& in_swapChainExtent, VkFormat& swapChainImageFormat, std::vector<VkImageView>& swapChainImageViews, uint32_t width, uint32_t height)
    {
        m_device = device;
        m_swapChainExtent = in_swapChainExtent;
        OnAttach();
    }

    void ImGuiLayer::Destroy()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        //vkDestroySampler(m_device, m_sampler, nullptr);

        vkDestroyDescriptorPool(m_device->GetVkDevice(), m_descriptorPool, nullptr);
        vkDestroyCommandPool(m_device->GetVkDevice(), m_commandPool, nullptr);
        vkDestroyRenderPass(m_device->GetVkDevice(), m_imGuiRenderPass, nullptr);


        for (int i = 0; i < m_imGuiFrameBuffers.size(); i++)
        {
            vkDestroyFramebuffer(m_device->GetVkDevice(), m_imGuiFrameBuffers[i], nullptr);
        }
    }

    void ImGuiLayer::SetStyle(uint32_t index)
    {
        switch (index)
        {
        case 0:
        {
            ImGuiStyle& style = ImGui::GetStyle();
            style = vulkanStyle;
            break;
        }
        case 1:
            ImGui::StyleColorsClassic();
            break;
        case 2:
            ImGui::StyleColorsDark();
            break;
        case 3:
            ImGui::StyleColorsLight();
            break;
        }
    }
	void ImGuiLayer::OnAttach()
	{
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;


        ImFontConfig config;
        config.OversampleH = 2;
        config.OversampleV = 1;
        config.GlyphExtraSpacing.x = 1.0f;

        float fontSize = 18.0f;// *2.0f;
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 24.0f, &config);\

        //ImGui::StyleColorsLight();
        vulkanStyle = ImGui::GetStyle();
        vulkanStyle.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.6f);
        vulkanStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.7f, 0.7f, 0.7f, 0.8f);
        vulkanStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.4f);
        vulkanStyle.Colors[ImGuiCol_Header] = ImVec4(1.0f, 1.0f, 1.0f, 0.4f);
        vulkanStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        SetStyle(0);

        this->io = &io;
	}
}
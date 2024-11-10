#include <ImGUILayer.h>
#include <Application.h>
#include <Vulkan/Initializer.hpp>

namespace Renderer
{
	ImGuiLayer::ImGuiLayer() :Layer("ImGui Layer")
	{
        ImGui::CreateContext();
	}

    void ImGuiLayer::OnDetach()
    {

    }

    void ImGuiLayer::InitVulkanResources(VkRenderPass renderPass, VkQueue copyQueue, const std::string& shadersPath, VkSampleCountFlagBits msaaSample)
    {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* fontData;
        int texWidth, texHeight;
        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
        VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

        
        // Create target image for copy
        Tools::CreateImage(m_device, texWidth, texHeight, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_fontImage, m_fontMemory);

        // Image view
        VkImageViewCreateInfo viewInfo = VulkanInitializer::ImageViewCreateInfo();
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

        //
        Tools::TransitionImageLayout(m_device, m_fontImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
        Tools::CopyBufferToImage(m_device, stagingBuffer.buffer, m_fontImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        Tools::TransitionImageLayout(m_device, m_fontImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        stagingBuffer.Destroy();

        // Font texture Sampler
        VkSamplerCreateInfo samplerInfo = VulkanInitializer::SamplerCreateInfo();
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        check_vk_result(vkCreateSampler(m_device->GetVkDevice(), &samplerInfo, nullptr, &m_sampler));

        //Here pass 2 is because m_swapChain have 2 images
        CreateImGuiDescriptorPool(2);

        // Descriptor set layout
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
        };
        VkDescriptorSetLayoutCreateInfo descriptorLayout = VulkanInitializer::DescriptorSetLayoutCreateInfo(setLayoutBindings);
        check_vk_result(vkCreateDescriptorSetLayout(m_device->GetVkDevice(), &descriptorLayout, nullptr, &m_descriptorSetLayout));

        // Descriptor set
        VkDescriptorSetAllocateInfo allocInfo = VulkanInitializer::DescriptorSetAllocateInfo(m_descriptorPool, &m_descriptorSetLayout, 1);
        check_vk_result(vkAllocateDescriptorSets(m_device->GetVkDevice(), &allocInfo, &m_descriptorSet));
        VkDescriptorImageInfo fontDescriptor = VulkanInitializer::DescriptorImageInfo(
            m_sampler,
            m_fontView,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
            VulkanInitializer::WriteDescriptorSet(m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &fontDescriptor)
        };
        vkUpdateDescriptorSets(m_device->GetVkDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

        //Pipeline Cache
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        check_vk_result(vkCreatePipelineCache(m_device->GetVkDevice(), &pipelineCacheCreateInfo, nullptr, &m_pipelineCache));

        //Pipeline layout
        //Push constant for UI Rendering parameters
        VkPushConstantRange pushConstantRange = VulkanInitializer::PushConstantRange(VK_SHADER_STAGE_VERTEX_BIT,sizeof(PushConstBlock),0);
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VulkanInitializer::PipelineLayoutCreateInfo(&m_descriptorSetLayout, 1);
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
        check_vk_result(vkCreatePipelineLayout(m_device->GetVkDevice(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = VulkanInitializer::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
        VkPipelineRasterizationStateCreateInfo rasterizationState = VulkanInitializer::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

        //Enable blending
        VkPipelineColorBlendAttachmentState blendAttachmentState{};
        blendAttachmentState.blendEnable = VK_TRUE;
        blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlendState =
            VulkanInitializer::PipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

        VkPipelineDepthStencilStateCreateInfo depthStencilState =
            VulkanInitializer::PipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

        VkPipelineViewportStateCreateInfo viewportState =
            VulkanInitializer::PipelineViewportStateCreateInfo(1, 1, 0);

        VkPipelineMultisampleStateCreateInfo multisampleState =
            VulkanInitializer::PipelineMultisampleStateCreateInfo(msaaSample);

        std::vector<VkDynamicState> dynamicStateEnables = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState =
            VulkanInitializer::PipelineDynamicStateCreateInfo(dynamicStateEnables);


        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

        VkGraphicsPipelineCreateInfo pipelineCreateInfo = VulkanInitializer::PipelineCreateInfo(m_pipelineLayout, renderPass);

        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineCreateInfo.pRasterizationState = &rasterizationState;
        pipelineCreateInfo.pColorBlendState = &colorBlendState;
        pipelineCreateInfo.pMultisampleState = &multisampleState;
        pipelineCreateInfo.pViewportState = &viewportState;
        pipelineCreateInfo.pDepthStencilState = &depthStencilState;
        pipelineCreateInfo.pDynamicState = &dynamicState;
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineCreateInfo.pStages = shaderStages.data();

        std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
            VulkanInitializer::VertexInputBindingDescription(0,sizeof(ImDrawVert),VK_VERTEX_INPUT_RATE_VERTEX),
        };
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
            VulkanInitializer::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),	// Location 0: Position
            VulkanInitializer::VertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),	// Location 1: UV
            VulkanInitializer::VertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),	// Location 0: Color
        };

        VkPipelineVertexInputStateCreateInfo vertexInputState = VulkanInitializer::PipelineVertexInputStateCreateInfo();
        vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
        vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
        vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
        vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

        pipelineCreateInfo.pVertexInputState = &vertexInputState;

        shaderStages[0] = Tools::LoadShader(m_device, shadersPath + "ui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT,m_vertexShaderModule);
        shaderStages[1] = Tools::LoadShader(m_device, shadersPath + "ui.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT,m_fragmentShaderModule);

        check_vk_result(vkCreateGraphicsPipelines(m_device->GetVkDevice(), m_pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipeline));
    }
     
    void ImGuiLayer::NewFrame()
    {
        ImGui::NewFrame();
        ImGui::SetWindowPos(ImVec2(20.f , 20.f ), ImGuiCond_FirstUseEver);
        ImGui::SetWindowSize(ImVec2(300.f , 300.f), ImGuiCond_Always);
        //ImGui::TextUnformatted("Debug Window");

        //ImGui::End();
        // Debug window
        ImGui::ShowDemoWindow();
        // Render to generate draw buffers
        ImGui::Render();
    }

    void ImGuiLayer::OnEvent(Event& e)
    {

    }

    void ImGuiLayer::UpdateImGUIEvent()
    {

    }

    void ImGuiLayer::DrawFrame(VkCommandBuffer commandBuffer)
    {
        ImGuiIO& io = ImGui::GetIO();
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        VkViewport viewport = VulkanInitializer::Viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        // UI scale and translate via push constants
        pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
        pushConstBlock.translate = glm::vec2(-1.0f);
        vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

        // Render commands
        ImDrawData* imDrawData = ImGui::GetDrawData();
        int32_t vertexOffset = 0;
        int32_t indexOffset = 0;

        if (imDrawData->CmdListsCount > 0) {

            VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

            for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
            {
                const ImDrawList* cmd_list = imDrawData->CmdLists[i];
                for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
                {
                    const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
                    VkRect2D scissorRect;
                    scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
                    scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
                    scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
                    scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
                    vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
                    vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
                    indexOffset += pcmd->ElemCount;
                }
                vertexOffset += cmd_list->VtxBuffer.Size;
            }
        }
    }

    void ImGuiLayer::UpdateBuffers()
    {
        ImDrawData* imDrawData = ImGui::GetDrawData();
        //Note: Alignment is done inside buffer creation
        VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
        VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

        if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
            return;
        }

        // Vertex buffer
        if ((m_vertexBuffer.buffer == VK_NULL_HANDLE) || (m_vertexCount != imDrawData->TotalVtxCount)) {
            m_vertexBuffer.Unmap();
            m_vertexBuffer.Destroy();
            check_vk_result(BufferUtils::CreateBuffer(m_device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &m_vertexBuffer, vertexBufferSize));
            m_vertexCount = imDrawData->TotalVtxCount;
            m_vertexBuffer.Map();
        }

        // Index buffer
        if ((m_indexBuffer.buffer == VK_NULL_HANDLE) || (m_indexCount < imDrawData->TotalIdxCount)) {
            m_indexBuffer.Unmap();
            m_indexBuffer.Destroy();
            check_vk_result(BufferUtils::CreateBuffer(m_device,VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &m_indexBuffer, indexBufferSize));
            m_indexCount = imDrawData->TotalIdxCount;
            m_indexBuffer.Map();
        }

        // Upload data
        ImDrawVert* vtxDst = (ImDrawVert*)m_vertexBuffer.mapped;
        ImDrawIdx* idxDst = (ImDrawIdx*)m_indexBuffer.mapped;

        for (int n = 0; n < imDrawData->CmdListsCount; n++) {
            const ImDrawList* cmd_list = imDrawData->CmdLists[n];
            memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtxDst += cmd_list->VtxBuffer.Size;
            idxDst += cmd_list->IdxBuffer.Size;
        }

        // Flush to make writes visible to GPU
        m_vertexBuffer.Flush();
        m_indexBuffer.Flush();

    }

    void ImGuiLayer::CreateImGuiDescriptorPool(uint32_t maxSets)
    {
        VkDescriptorPoolSize imGui_pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = maxSets;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(imGui_pool_sizes);
        pool_info.pPoolSizes = imGui_pool_sizes;
        VkResult err = vkCreateDescriptorPool(m_device->GetVkDevice(), &pool_info, nullptr, &m_descriptorPool);
        check_vk_result(err);
    }

    void ImGuiLayer::InitImGUIAttribute(Device*  device, VkExtent2D& in_swapChainExtent, VkRenderPass renderPass, VkQueue copyQueue, const std::string& shadersPath, VkSampleCountFlagBits msaaSample)
    {
        m_device = device;
        m_swapChainExtent = in_swapChainExtent;
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(m_swapChainExtent.width, m_swapChainExtent.height);
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
        InitVulkanResources(renderPass,copyQueue,shadersPath, msaaSample);
    }

    void ImGuiLayer::Destroy()
    {
        ImGui::DestroyContext();
        m_vertexBuffer.Destroy();
        m_indexBuffer.Destroy();
        vkDestroyImage(m_device->GetVkDevice(), m_fontImage, nullptr);
        vkDestroyImageView(m_device->GetVkDevice(), m_fontView, nullptr);
        vkFreeMemory(m_device->GetVkDevice(), m_fontMemory, nullptr);
        vkDestroySampler(m_device->GetVkDevice(), m_sampler, nullptr);
        vkDestroyDescriptorPool(m_device->GetVkDevice(), m_descriptorPool, nullptr);
        vkDestroyCommandPool(m_device->GetVkDevice(), m_commandPool, nullptr);

        vkDestroyShaderModule(m_device->GetVkDevice(), m_vertexShaderModule,nullptr);
        vkDestroyShaderModule(m_device->GetVkDevice(), m_fragmentShaderModule, nullptr);

        vkDestroyPipeline(m_device->GetVkDevice(), m_pipeline, nullptr);
        vkDestroyPipelineCache(m_device->GetVkDevice(), m_pipelineCache, nullptr);
        vkDestroyPipelineLayout(m_device->GetVkDevice(), m_pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(m_device->GetVkDevice(), m_descriptorSetLayout, nullptr);
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
}
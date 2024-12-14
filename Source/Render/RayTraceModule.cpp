#include <Render/RayTraceModule.h>

namespace Renderer
{
    RayTraceModule::RayTraceModule(std::shared_ptr<Device> device):m_device(device){}

    //Compute RayTrace Pipeline Functions
    void RayTraceModule::CreateRayTracePipeline()
    {
        CreateRayTraceDescriptorSet();

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VulkanInitializer::PipelineLayoutCreateInfo(&m_rayTraceResources.descriptorSetLayout, 1);
        check_vk_result(vkCreatePipelineLayout(m_device->GetVkDevice(), &pipelineLayoutCreateInfo, nullptr, &m_rayTraceResources.pipelineLayout));

        VkComputePipelineCreateInfo computePipelineCreateInfo = VulkanInitializer::ComputePipelineCreateInfo(m_rayTraceResources.pipelineLayout, 0);
        auto compShaderCode = Tools::ReadFile("./Shaders/rayTrace.spv");

        VkShaderModule computeShaderModule = Tools::CreateShaderModule(m_device.get(), compShaderCode);

        VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
        computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeShaderStageInfo.module = computeShaderModule;
        computeShaderStageInfo.pName = "main";
        computePipelineCreateInfo.stage = computeShaderStageInfo;
        check_vk_result(vkCreateComputePipelines(m_device->GetVkDevice(), nullptr, 1, &computePipelineCreateInfo, nullptr, &m_rayTraceResources.pipeline));

        CreateRayTraceCommandPool(&m_rayTraceResources.commandPool);

        // Create a command buffer for compute operations
        VkCommandBufferAllocateInfo cmdBufAllocateInfo = VulkanInitializer::CommandBufferAllocateInfo(m_rayTraceResources.commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
        check_vk_result(vkAllocateCommandBuffers(m_device->GetVkDevice(), &cmdBufAllocateInfo, &m_rayTraceResources.commandBuffer));

        // Fence for compute CB sync
        VkFenceCreateInfo fenceCreateInfo = VulkanInitializer::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
        check_vk_result(vkCreateFence(m_device->GetVkDevice(), &fenceCreateInfo, nullptr, &m_rayTraceResources.fence));
    }

    void RayTraceModule::CreateRayTraceCommandPool(VkCommandPool* commandPool)
    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = m_device->GetInstance()->GetQueueFamilyIndices()[QueueFlags::Compute];
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        check_vk_result(vkCreateCommandPool(m_device->GetVkDevice(), &cmdPoolInfo, nullptr, commandPool));
    }

    void RayTraceModule::RecordComputeCommandBuffer()
    {
        VkCommandBufferBeginInfo cmdCommandBufferBeginInfo = VulkanInitializer::CommandBufferBeginInfo();
        check_vk_result(vkBeginCommandBuffer(m_rayTraceResources.commandBuffer, &cmdCommandBufferBeginInfo));

        // Image memory barrier to make sure that compute shader writes are finished before sampling from the texture
        VkImageMemoryBarrier imageMemoryBarrier = { };
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.image = m_storageImage.m_image;
        imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 };

        //Acquire barrier for graphics queue
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.srcQueueFamilyIndex = m_device->GetQueueIndex(QueueFlags::Compute);
        imageMemoryBarrier.dstQueueFamilyIndex = m_device->GetQueueIndex(QueueFlags::Graphics);
        vkCmdPipelineBarrier(m_rayTraceResources.commandBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);

        vkCmdBindPipeline(m_rayTraceResources.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_rayTraceResources.pipeline);
        vkCmdBindDescriptorSets(m_rayTraceResources.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_rayTraceResources.pipelineLayout, 0, 1, &m_rayTraceResources.descriptorSet, 0, 0);

        vkCmdDispatch(m_rayTraceResources.commandBuffer, m_storageImage.width / 16, m_storageImage.height / 16, 1);

        // Release barrier from compute queue
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = 0;
        imageMemoryBarrier.srcQueueFamilyIndex = m_device->GetQueueIndex(QueueFlags::Compute);
        imageMemoryBarrier.dstQueueFamilyIndex = m_device->GetQueueIndex(QueueFlags::Graphics);
        vkCmdPipelineBarrier(
            m_rayTraceResources.commandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);

        vkEndCommandBuffer(m_rayTraceResources.commandBuffer);
    }

    void RayTraceModule::CreateDescriptorPool(VkDescriptorPool& descriptorPool)
    {
        std::vector<VkDescriptorPoolSize> poolSizes = {
              VulkanInitializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
              VulkanInitializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4),
              VulkanInitializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
              VulkanInitializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2),
        };
        //maxSet can be changed 
        VkDescriptorPoolCreateInfo descriptorPoolInfo = VulkanInitializer::DescriptorPoolCreateInfo(poolSizes, 3);
        check_vk_result(vkCreateDescriptorPool(m_device->GetVkDevice(), &descriptorPoolInfo, nullptr, &descriptorPool));

        m_descriptorPool = descriptorPool;
    }

    void RayTraceModule::CreateRayTraceDescriptorSet()
    {
        // The compute pipeline uses one set and four bindings
        // Binding 0: Storage image for raytraced output
        // Binding 1: Uniform buffer with parameters
        // Binding 2: Shader storage buffer with scene object definitions
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
        {
            VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
            VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
            VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2),
        };

        VkDescriptorSetLayoutCreateInfo descriptorLayout = VulkanInitializer::DescriptorSetLayoutCreateInfo(setLayoutBindings);
        check_vk_result(vkCreateDescriptorSetLayout(m_device->GetVkDevice(), &descriptorLayout, nullptr, &m_rayTraceResources.descriptorSetLayout));

        VkDescriptorSetAllocateInfo allocInfo = VulkanInitializer::DescriptorSetAllocateInfo(m_descriptorPool, &m_rayTraceResources.descriptorSetLayout, 1);

        check_vk_result(vkAllocateDescriptorSets(m_device->GetVkDevice(), &allocInfo, &m_rayTraceResources.descriptorSet));
        std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
            VulkanInitializer::WriteDescriptorSet(m_rayTraceResources.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &m_storageImage.descriptor),
            VulkanInitializer::WriteDescriptorSet(m_rayTraceResources.descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &m_rayTraceResources.uniformBuffer.descriptor),
            VulkanInitializer::WriteDescriptorSet(m_rayTraceResources.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, &m_rayTraceResources.objectStorageBuffer.descriptor),
        };
        vkUpdateDescriptorSets(m_device->GetVkDevice(), static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, nullptr);
    }

    void RayTraceModule::CreateRayTraceStorageImage(uint32_t width, uint32_t height)
    {
        const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(m_device->GetInstance()->GetPhysicalDevice(), format, &formatProperties);
        if (!formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
        {
            LOG_CLIENT_ERROR("Raytrace required storage format is not supported");
            return;
        }
        m_storageImage.width = width;
        m_storageImage.height = height;

        Tools::CreateImage(m_device.get(), m_storageImage.width, m_storageImage.height, 1, VK_SAMPLE_COUNT_1_BIT,
            format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_storageImage.m_image, m_storageImage.m_imageDeviceMemory);

        m_storageImage.m_imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        //Compute Shader have different  layout mask(need to write a function to handle it differently)
        Tools::TransitionImageLayout(m_device.get(), m_storageImage.m_image, VK_IMAGE_LAYOUT_UNDEFINED, m_storageImage.m_imageLayout, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        VkCommandBuffer cmd = Tools::CreateCommandBuffer(m_device.get(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_device->GetGraphicCommandPool(), true);

        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.image = m_storageImage.m_image;
        imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = 0;
        imageMemoryBarrier.srcQueueFamilyIndex = m_device->GetQueueIndex(QueueFlags::Graphics);
        imageMemoryBarrier.dstQueueFamilyIndex = m_device->GetQueueIndex(QueueFlags::Compute);
        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);

        Tools::EndCommandBuffer(m_device.get(), cmd, m_device->GetGraphicCommandPool(), QueueFlags::Graphics);

        // Create sampler
        VkSamplerCreateInfo sampler = VulkanInitializer::SamplerCreateInfo();
        sampler.magFilter = VK_FILTER_LINEAR;
        sampler.minFilter = VK_FILTER_LINEAR;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampler.addressModeV = sampler.addressModeU;
        sampler.addressModeW = sampler.addressModeU;
        sampler.mipLodBias = 0.0f;
        sampler.maxAnisotropy = 1.0f;
        sampler.compareOp = VK_COMPARE_OP_NEVER;
        sampler.minLod = 0.0f;
        sampler.maxLod = 0.0f;
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        check_vk_result(vkCreateSampler(m_device->GetVkDevice(), &sampler, nullptr, &m_storageImage.m_sampler));

        // Create image view
        VkImageViewCreateInfo view = VulkanInitializer::ImageViewCreateInfo();
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = format;
        view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        view.image = m_storageImage.m_image;
        check_vk_result(vkCreateImageView(m_device->GetVkDevice(), &view, nullptr, &m_storageImage.m_imageView));

        m_storageImage.UpdateDescriptor();
        m_storageImage.m_device = m_device;
    }

    void RayTraceModule::UpdateUniformBuffers(const Camera& cam)
    {
        glm::vec4 camPos = cam.GetPosition();
        glm::vec4 camForward = cam.GetForwardVector();
        m_rayTraceUniform.camPos = glm::vec3(camPos.x, camPos.y, camPos.z);
        m_rayTraceUniform.cam_lookat = glm::vec3(camForward.x, camForward.y, camForward.z);
        m_rayTraceUniform.fov = cam.GetFOV();
        m_rayTraceUniform.aspectRatio = cam.GetAspectRatio();
        
        check_vk_result(m_rayTraceResources.uniformBuffer.Map());
        memcpy(m_rayTraceResources.uniformBuffer.mapped, &m_rayTraceUniform, sizeof(RayTraceUniformData));
        m_rayTraceResources.uniformBuffer.Unmap();
    }

    Texture2D RayTraceModule::GetStorageImage() const
    {
        return m_storageImage;
    }


    ComputeResource RayTraceModule::GetRayTraceComputeResource() const
    {
        return m_rayTraceResources;
    }

    void RayTraceModule::DestroyVKResources()
    {
        vkDestroyPipeline(m_device->GetVkDevice(), m_rayTraceResources.pipeline,nullptr);
        vkDestroyPipelineLayout(m_device->GetVkDevice(),m_rayTraceResources.pipelineLayout,nullptr);
        vkDestroyDescriptorSetLayout(m_device->GetVkDevice(), m_rayTraceResources.descriptorSetLayout, nullptr);
        vkDestroyFence(m_device->GetVkDevice(), m_rayTraceResources.fence, nullptr);
        vkDestroyCommandPool(m_device->GetVkDevice(), m_rayTraceResources.commandPool, nullptr);
        m_storageImage.DestroyVKResources();
    }
}

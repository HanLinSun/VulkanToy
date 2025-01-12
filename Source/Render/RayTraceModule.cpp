#include <Render/RayTraceModule.h>

#include <Vulkan/BufferUtils.h>

namespace Renderer
{
    RayTraceModule::RayTraceModule(std::shared_ptr<Device> device):m_device(device),isTriangleGPUBufferAlloc(false),
        isMaterialGPUBufferAlloc(false),isSphereGPUBufferAlloc(false), isLightGPUBufferAlloc(false) {}

    //Compute RayTrace Pipeline Functions
    void RayTraceModule::CreateRayTracePipeline()
    {
        CreateRayTraceDescriptorSet();

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VulkanInitializer::PipelineLayoutCreateInfo(&m_rayTraceResources.descriptorSetLayout, 1);
        check_vk_result(vkCreatePipelineLayout(m_device->GetVkDevice(), &pipelineLayoutCreateInfo, nullptr, &m_rayTraceResources.pipelineLayout));

        VkComputePipelineCreateInfo computePipelineCreateInfo = VulkanInitializer::ComputePipelineCreateInfo(m_rayTraceResources.pipelineLayout, 0);
        auto compShaderCode = Tools::ReadFile("./Shaders/glsl/rayTrace.spv");

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

        vkDestroyShaderModule(m_device->GetVkDevice(), computeShaderModule, nullptr);
    }

    inline void RayTraceModule::AddGPUWriteDescriptorSet(bool isGPUBufferAlloc, std::vector<VkWriteDescriptorSet>& computeWriteDescriptorSets, VkDescriptorType type, VkDescriptorBufferInfo* bufferInfo, int binding)
    {
        if (isGPUBufferAlloc)
        {
            VkWriteDescriptorSet descriptorSet = VulkanInitializer::WriteDescriptorSet(m_rayTraceResources.descriptorSet, type, binding, bufferInfo);
            computeWriteDescriptorSets.push_back(descriptorSet);
        }
    }

    void RayTraceModule::SetRenderScene(Scene* scene)
    {
        m_scene = scene;
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

        vkCmdDispatch(m_rayTraceResources.commandBuffer, m_storageImage.width / 32, m_storageImage.height / 32, 1);

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

    void RayTraceModule::CreateRayTraceComputeDescriptorPool()
    {
        std::vector<VkDescriptorPoolSize> poolSizes = {
              VulkanInitializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
              VulkanInitializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
              VulkanInitializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1),
              VulkanInitializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1),
              VulkanInitializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1),
              VulkanInitializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_scene->GetTextures().size()!=0? m_scene->GetTextures().size():1),
              VulkanInitializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1),
        };
        //maxSet can be changed 
        VkDescriptorPoolCreateInfo descriptorPoolInfo = VulkanInitializer::DescriptorPoolCreateInfo(poolSizes, m_scene->GetTextures().size() + 7);
        check_vk_result(vkCreateDescriptorPool(m_device->GetVkDevice(), &descriptorPoolInfo, nullptr, &m_rayTraceComputeDescriptorPool));
    }

    void RayTraceModule::CreateUniformBuffer()
    {
        VkDeviceSize bufferSize = sizeof(RayTraceUniformData);
        BufferUtils::CreateBuffer(m_device.get(),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &m_rayTraceResources.uniformBuffer,bufferSize);
    }

    void RayTraceModule::CreateRenderStorageBuffer()
    {
        //Binding =2, triangles
        std::vector<Triangle> scene_triangles = m_scene->GetTriangles();
        if (scene_triangles.size() != 0)
        {
            BufferUtils::CreateGPUBuffer<Triangle>(m_device.get(), scene_triangles.data(), scene_triangles.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &m_trianglesGPUBuffer);
            isTriangleGPUBufferAlloc = true;
        }
    
        //Binding = 3, materials
        std::vector<PBRMaterialData> scene_material = m_scene->GeneratePBRMaterialData();
        if (scene_material.size() != 0)
        {
            BufferUtils::CreateGPUBuffer<PBRMaterialData>(m_device.get(), scene_material.data(), scene_material.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &m_materialGPUBuffer);
            isMaterialGPUBufferAlloc = true;
        }
      
        //Binding =4, Spheres
        std::vector<Sphere> scene_spheres = m_scene->GetSpheres();
        if (scene_spheres.size() != 0)
        {
            BufferUtils::CreateGPUBuffer<Sphere>(m_device.get(), scene_spheres.data(), scene_spheres.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &m_sphereGPUBuffer);
            isSphereGPUBufferAlloc = true;
        }

        //Binding =5, Textures, already have texture arrays. 

        //Binding =6, Lights
        std::vector<LightGPU> scene_lights = m_scene->GetLights();
        if (scene_lights.size() != 0)
        {
            BufferUtils::CreateGPUBuffer<LightGPU>(m_device.get(), scene_lights.data(), scene_lights.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &m_lightGPUBuffer);
            isLightGPUBufferAlloc = true;
        }

        m_scene->GenerateBVHObjectArray();
        std::vector<BVHObject> bvhObjects = m_scene->GetBVHObjectArray();
        std::vector<BVHNodeGPU> bvhNodes = BVHBuildTool::BuildBVHGPUNode(bvhObjects);
        //Binding =7,BVHNodes
        if (bvhNodes.size() != 0)
        {
            BufferUtils::CreateGPUBuffer<BVHNodeGPU>(m_device.get(), bvhNodes.data(), bvhNodes.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &m_BVHNodeGPUBuffer);
            isBVHNodeBufferAlloc = true;
        }
            
    }

    void RayTraceModule::CreateRayTraceDescriptorSet()
    {
        // The compute pipeline uses one set and four bindings
        // Binding 0: Storage image for raytraced output
        // Binding 1: Uniform buffer with parameters
        // Binding 2: Triangle buffer
        // Binding 3: Material buffer
        // Binding 4: Sphere buffer
        // Binding 5: Texture buffer
        // Binding 6: Light buffer
        // Binding 7: AABB buffer

        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
        {
            VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0),
            VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
            VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2),
            VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 3),
            VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 4),
            VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, 5, m_scene->GetTextures().size()!=0? m_scene->GetTextures().size():1),
            VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 6),
            VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 7),
        };

        if (m_scene->GetTextures().size() >= 0)
        {
            VkDescriptorSetLayoutBinding textureLayoutBinding = VulkanInitializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, 5, m_scene->GetTextures().size());
            setLayoutBindings.push_back(textureLayoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo descriptorLayout = VulkanInitializer::DescriptorSetLayoutCreateInfo(setLayoutBindings);
        check_vk_result(vkCreateDescriptorSetLayout(m_device->GetVkDevice(), &descriptorLayout, nullptr, &m_rayTraceResources.descriptorSetLayout));

        VkDescriptorSetAllocateInfo allocInfo = VulkanInitializer::DescriptorSetAllocateInfo(m_rayTraceComputeDescriptorPool, &m_rayTraceResources.descriptorSetLayout, 1);

        check_vk_result(vkAllocateDescriptorSets(m_device->GetVkDevice(), &allocInfo, &m_rayTraceResources.descriptorSet));

        std::vector<VkDescriptorImageInfo> imageInfos(m_scene->GetTextures().size());

        for (size_t t = 0; t < imageInfos.size(); ++t)
        {
            imageInfos[t].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[t].imageView = m_scene->GetTexture(t)->m_imageView;
            imageInfos[t].sampler = m_scene->GetTexture(t)->m_sampler;
        }

        //Modify this to loop update
        
        std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
            VulkanInitializer::WriteDescriptorSet(m_rayTraceResources.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &m_storageImage.descriptor),
            VulkanInitializer::WriteDescriptorSet(m_rayTraceResources.descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &m_rayTraceResources.uniformBuffer.descriptor),
           // VulkanInitializer::WriteDescriptorSet(m_rayTraceResources.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, &m_trianglesGPUBuffer.descriptor),
          //  VulkanInitializer::WriteDescriptorSet(m_rayTraceResources.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, &m_materialGPUBuffer.descriptor),
           // VulkanInitializer::WriteDescriptorSet(m_rayTraceResources.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4 ,&m_sphereGPUBuffer.descriptor),
          //  VulkanInitializer::WriteDescriptorSet(m_rayTraceResources.descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,5, imageInfos.data(),static_cast<uint32_t>(imageInfos.size())),
            //VulkanInitializer::WriteDescriptorSet(m_rayTraceResources.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6 ,&m_lightGPUBuffer.descriptor),
            //VulkanInitializer::WriteDescriptorSet(m_rayTraceResources.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 7  ,&m_BVHNodeGPUBuffer.descriptor),
        };

        AddGPUWriteDescriptorSet(isTriangleGPUBufferAlloc, computeWriteDescriptorSets, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &m_trianglesGPUBuffer.descriptor, 2);
        AddGPUWriteDescriptorSet(isMaterialGPUBufferAlloc, computeWriteDescriptorSets, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &m_materialGPUBuffer.descriptor, 3);
        AddGPUWriteDescriptorSet(isSphereGPUBufferAlloc, computeWriteDescriptorSets, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &m_sphereGPUBuffer.descriptor, 4);
        AddGPUWriteDescriptorSet(isLightGPUBufferAlloc, computeWriteDescriptorSets, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &m_lightGPUBuffer.descriptor, 6);
        AddGPUWriteDescriptorSet(isBVHNodeBufferAlloc, computeWriteDescriptorSets, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &m_BVHNodeGPUBuffer.descriptor, 7);

        if (imageInfos.size()!= 0)
        {
                VkWriteDescriptorSet descriptorWriteSet{};
                descriptorWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWriteSet.dstSet = m_rayTraceResources.descriptorSet;
                descriptorWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWriteSet.dstBinding = 5;
                descriptorWriteSet.pImageInfo = imageInfos.data();
                descriptorWriteSet.descriptorCount = static_cast<uint32_t>(imageInfos.size());
                computeWriteDescriptorSets.push_back(descriptorWriteSet);
        }

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
        // subresource level can not be 0, must bigger than 1. if mipmap fill the mipmap level; else use 1
        Tools::TransitionImageLayout(m_device.get(), m_storageImage.m_image, VK_IMAGE_LAYOUT_UNDEFINED, m_storageImage.m_imageLayout, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

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

    void RayTraceModule::UpdateUniformBuffer(Camera* cam)
    {
        glm::vec4 camPos = cam->GetPosition();
        glm::vec4 camForward = cam->GetLookTarget();
        glm::vec4 camLookAt = cam->GetLookTarget();
        glm::vec3 cameraUp = cam->GetUpVector();;

        m_rayTraceUniform.camProjectionMatrix = cam->GetProjectionMatrix();
        m_rayTraceUniform.camViewMatrix = cam->GetViewmatrix();
        m_rayTraceUniform.aperture = cam->GetAperture();
        m_rayTraceUniform.focalDistance = cam->GetFocalDistance();
        m_rayTraceUniform.aperture = cam->GetAspectRatio();
        m_rayTraceUniform.lightNums = 2;
        m_rayTraceUniform.samplePerPixel = 5;
        m_rayTraceUniform.maxRecursiveDepth =15;
        m_rayTraceUniform.triangleNums = m_scene->GetTriangles().size();
        m_rayTraceUniform.sphereNums = m_scene->GetSpheres().size();
                
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
        vkDestroyDescriptorPool(m_device->GetVkDevice(), m_rayTraceComputeDescriptorPool, nullptr);
        m_rayTraceResources.uniformBuffer.Destroy();

        if (isTriangleGPUBufferAlloc) m_trianglesGPUBuffer.Destroy();
        if (isSphereGPUBufferAlloc) m_sphereGPUBuffer.Destroy();
        if (isLightGPUBufferAlloc) m_lightGPUBuffer.Destroy();
        if (isMaterialGPUBufferAlloc) m_materialGPUBuffer.Destroy();
        if (isBVHNodeBufferAlloc) m_BVHNodeGPUBuffer.Destroy();

        m_storageImage.DestroyVKResources();
    }
}


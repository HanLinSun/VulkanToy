#include <Vulkan/Texture.h>
#include <Vulkan/BufferUtils.h>
#include "Tools.h"
#include "Log.h"
#include <Vulkan/Initializer.hpp>
void Texture::UpdateDescriptor()
{
	descriptor.sampler = m_sampler;
	descriptor.imageView = m_imageView;
	descriptor.imageLayout = m_imageLayout;
}

void Texture::DestroyVKResources()
{
	vkDestroyImageView(m_device->GetVkDevice(),m_imageView, nullptr);
	vkDestroyImage(m_device->GetVkDevice(), m_image, nullptr);
	if (m_sampler)
	{
		vkDestroySampler(m_device->GetVkDevice(), m_sampler, nullptr);
	}
	vkFreeMemory(m_device->GetVkDevice(), m_imageDeviceMemory, nullptr);
}

int Texture2D::LoadFromFile(std::string filename, VkFormat format, std::shared_ptr<Device> device, VkImageUsageFlags  imageUsageFlags,
	VkImageLayout imageLayout , bool forceLinear)
{ 
        this->m_device = device;
    
	    int texWidth, texHeight;
		int texChannels;
        stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (pixels == nullptr)
        {
            LOG_CORE_ERROR("pixel data is null, " + filename + " might be empty, loading stopped.");
            return -1;
        }
        
        this->fileName = filename;
	    this->m_pixels = pixels;
        this->width = texWidth;
        this->height = texHeight;
        
        //Here I force all texture to be RGBA format and have 4 channels
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        
        this->mipLevels = mipLevels;

       VkBuffer stagingBuffer;
       VkDeviceMemory stagingBufferMemory;
	   BufferUtils::CreateBuffer(m_device.get(), imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

       void* data;
       vkMapMemory(m_device->GetVkDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
       memcpy(data, this->m_pixels, static_cast<size_t>(imageSize));
       vkUnmapMemory(m_device->GetVkDevice(), stagingBufferMemory);

       stbi_image_free(this->m_pixels);

       Tools::CreateImage(m_device.get(), this->width, this->height, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->m_image, this->m_imageDeviceMemory);
       Tools::TransitionImageLayout(m_device.get(), this->m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels,VK_PIPELINE_STAGE_HOST_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT);
       Tools::CopyBufferToImage(m_device.get(), stagingBuffer, this->m_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

       vkDestroyBuffer(m_device->GetVkDevice(), stagingBuffer, nullptr);
       vkFreeMemory(m_device->GetVkDevice(), stagingBufferMemory, nullptr);

       Tools::GenerateMipmaps(m_device.get(), this->m_image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

       // Create a default sampler

       VkPhysicalDeviceProperties properties{};
       vkGetPhysicalDeviceProperties(device->GetInstance()->GetPhysicalDevice(), &properties);
       
      VkSamplerCreateInfo samplerInfo{};
       samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
       samplerInfo.magFilter = VK_FILTER_LINEAR;
       samplerInfo.minFilter = VK_FILTER_LINEAR;
       samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
       samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
       samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
       samplerInfo.anisotropyEnable = VK_TRUE;
       samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
       samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
       samplerInfo.unnormalizedCoordinates = VK_FALSE;
       samplerInfo.compareEnable = VK_FALSE;
       samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
       samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
       samplerInfo.minLod = 0.0f;
       samplerInfo.maxLod = static_cast<float>(mipLevels);
       samplerInfo.mipLodBias = 0.0f;
       
      if (vkCreateSampler(m_device->GetVkDevice(), &samplerInfo, nullptr, &this->m_sampler) != VK_SUCCESS) {
           throw std::runtime_error("failed to create texture sampler!");
       }

       // Create image view
       // Textures are not directly accessed by the shaders and
       // are abstracted by image views containing additional
       // information and sub resource ranges
       VkImageViewCreateInfo viewCreateInfo = {};
       viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
       viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
       viewCreateInfo.format = format;
       viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

       viewCreateInfo.subresourceRange.levelCount = mipLevels;
       viewCreateInfo.image = this->m_image;
       if (vkCreateImageView(this->m_device->GetVkDevice(), &viewCreateInfo, nullptr, &this->m_imageView)!=VK_SUCCESS)
       {
           throw std::runtime_error("failed to create texture image view!");
       }

       // Update descriptor image info member that can be used for setting up descriptor sets
       UpdateDescriptor();
       return 0;
}


void Texture2D::LoadFromBuffer(void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight, std::shared_ptr<Device> device, VkQueue copyQueue,
	VkFilter filter, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout)
{

}

void Texture2DArray::LoadFromFile(std::string filename, VkFormat format, std::shared_ptr<Device> device, VkQueue copyQueue, VkImageUsageFlags  imageUsageFlags, VkImageLayout imageLayout)
{

}

int TextureCubeMap::LoadFromFiles(std::vector<std::string> filenames, VkFormat format, std::shared_ptr<Device> device, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout)
{
    this->m_device = device;
    int texWidth, texHeight;
    int texChannels;

    for (int i = 0; i < 6; i++)
    {
        stbi_uc* face_pixels = stbi_load(filenames[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (face_pixels == nullptr)
        {
            LOG_CORE_ERROR("pixel data is null, " + filenames[i] + " might be empty, loading stopped.");
            return -1;
        }
        m_facePixelData[i] = face_pixels;
    }

    this->width = texWidth;
    this->height = texHeight;

    unsigned char* cubemapData = (unsigned char*)malloc(6 * texWidth * texHeight * 4);
    for (int i = 0; i < 6; i++)
    {
        memcpy(cubemapData + i * (texWidth * texHeight * 4), m_facePixelData[i], texWidth * texHeight * 4);
        stbi_image_free(m_facePixelData[i]);
    }

    VkDeviceSize imageSize = 6 * texWidth * texHeight * 4;
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    this->mipLevels = mipLevels;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    BufferUtils::CreateBuffer(m_device.get(), imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device->GetVkDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, cubemapData, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device->GetVkDevice(), stagingBufferMemory);

    Tools::CreateCubeMapImage(m_device.get(), this->width, this->height, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->m_image, this->m_imageDeviceMemory);
   

    VkCommandBuffer copyCmd = Tools::CreateCommandBuffer(m_device.get(), VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_device->GetGraphicCommandPool(), true);
    std::vector<VkBufferImageCopy> bufferCopyRegions;

    size_t offset = 0;
    for (uint32_t face = 0; face < 6; face++)
    {
        for (uint32_t level = 0; level < mipLevels; level++)
        {
            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = level;
            bufferCopyRegion.imageSubresource.baseArrayLayer = face;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width =texWidth >> level;
            bufferCopyRegion.imageExtent.height =texHeight >> level;
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = offset;
            bufferCopyRegions.push_back(bufferCopyRegion);

            offset += (texWidth >> level) * (texHeight >> level) * 4;
        }
    }

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipLevels;
    //Once time loading have 6 textures, and only have 1 layer
    subresourceRange.layerCount = 6;

    Tools::TransitionImageLayout(m_device.get(), this->m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, VK_PIPELINE_STAGE_HOST_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT );

    vkCmdCopyBufferToImage(
        copyCmd,
        stagingBuffer,
       this->m_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        static_cast<uint32_t>(bufferCopyRegions.size()),
        bufferCopyRegions.data()
    );

    this->m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    Tools::TransitionImageLayout(m_device.get(), this->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, this->m_imageLayout, mipLevels,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    Tools::EndCommandBuffer(m_device.get(), copyCmd, m_device->GetGraphicCommandPool(), QueueFlags::Graphics);

    VkSamplerCreateInfo samplerInfo = VulkanInitializer::SamplerCreateInfo();
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipLevels);
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.maxAnisotropy = 1.0f;

    if (vkCreateSampler(m_device->GetVkDevice(), &samplerInfo, nullptr, &this->m_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    VkImageViewCreateInfo view = VulkanInitializer::ImageViewCreateInfo();
    view.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    view.format = format;
    view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    view.subresourceRange.layerCount = 6;
    view.subresourceRange.levelCount =this->mipLevels;
    view.image = this->m_image;

    // Clean up staging resources
    vkFreeMemory(device->GetVkDevice(), stagingBufferMemory, nullptr);
    vkDestroyBuffer(device->GetVkDevice(), stagingBuffer, nullptr);
}
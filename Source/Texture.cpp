#include "Vulkan/Texture.h"
#include "Vulkan/BufferUtils.h"
#include "Tools.h"
#include "Log.h"
void Texture::UpdateDescriptor()
{
	descriptor.sampler = m_sampler;
	descriptor.imageView = m_imageView;
	descriptor.imageLayout = m_imageLayout;
}

void Texture::Destroy()
{
	vkDestroyImageView(m_device->GetVkDevice(),m_imageView, nullptr);
	vkDestroyImage(m_device->GetVkDevice(), m_image, nullptr);
	if (m_sampler)
	{
		vkDestroySampler(m_device->GetVkDevice(), m_sampler, nullptr);
	}
	vkFreeMemory(m_device->GetVkDevice(), m_imageDeviceMemory, nullptr);
}

int Texture2D::LoadFromFile(std::string filename, VkFormat format, Device* device, VkImageUsageFlags  imageUsageFlags,
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
        
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        
        this->mipLevels = mipLevels;

       VkBuffer stagingBuffer;
       VkDeviceMemory stagingBufferMemory;
	   BufferUtils::CreateBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

       void* data;
       vkMapMemory(m_device->GetVkDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
       memcpy(data, this->m_pixels, static_cast<size_t>(imageSize));
       vkUnmapMemory(m_device->GetVkDevice(), stagingBufferMemory);

       stbi_image_free(this->m_pixels);

       Tools::CreateImage(m_device, this->width, this->height, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->m_image, this->m_imageDeviceMemory);
       Tools::TransitionImageLayout(m_device, this->m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
       Tools::CopyBufferToImage(m_device,stagingBuffer, this->m_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

       vkDestroyBuffer(m_device->GetVkDevice(), stagingBuffer, nullptr);
       vkFreeMemory(m_device->GetVkDevice(), stagingBufferMemory, nullptr);

       Tools::GenerateMipmaps(m_device,this->m_image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

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


void Texture2D::LoadFromBuffer(void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight, Device* device, VkQueue copyQueue,
	VkFilter filter, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout)
{

}

void Texture2DArray::LoadFromFile(std::string filename, VkFormat format, Device* device, VkQueue copyQueue, VkImageUsageFlags  imageUsageFlags, VkImageLayout imageLayout)
{

}

#pragma once
#include "Device.h"
#include "Instance.h"
#include <RendererInclude.h>
#include <stb_image.h>

class Texture
{
public:

	Device* m_device;
	VkImage  m_image;
	VkImageLayout m_imageLayout;
	VkDeviceMemory m_imageDeviceMemory;
	VkImageView m_imageView;
	uint32_t  width, height;
	uint32_t  mipLevels;
	uint32_t  layerCount;
	VkDescriptorImageInfo descriptor;
	VkSampler m_sampler;
	//stb_image
	stbi_uc* m_pixels;

	std::string fileName;
	void UpdateDescriptor();
	void Destroy();
};

class Texture2D : public Texture
{
public:
	int LoadFromFile(std::string filename,VkFormat format,Device* device,	VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,bool forceLinear = false);

	void LoadFromBuffer(void* buffer,	VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth,uint32_t texHeight, Device* device,	VkQueue copyQueue,
		VkFilter filter = VK_FILTER_LINEAR,	VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
};

class Texture2DArray : public Texture
{
public:
	void LoadFromFile(std::string filename,VkFormat format,Device* device,	VkQueue copyQueue,	VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
};

class TextureCubeMap : public Texture
{
public:
	void LoadFromFile(std::string filename,VkFormat format,Device* device,	VkQueue copyQueue,	VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
};
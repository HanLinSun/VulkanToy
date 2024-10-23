#pragma once
#include <Vulkan/vulkan.h>
#include <Vulkan/Device.h>
#include <Vulkan/Instance.h>


#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << vks::tools::errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}

namespace Tools
{

	VkBool32 GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat);
	// Same as getSupportedDepthFormat but will only select formats that also have stencil
	VkBool32 GetSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat);

	// Returns tru a given format support LINEAR filtering
	VkBool32 FormatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling);

	VkBool32 FormatHasStencil(VkFormat format);

	VkCommandBuffer CreateCommandBuffer(Device* device, VkCommandBufferLevel level, VkCommandPool pool, bool begin);
	void CreateCommandPool(Device* device, QueueFlags flag, VkCommandPoolCreateFlags createFlags, VkCommandPool* cmdPool);
	void CopyBufferToImage(Device* device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void GenerateMipmaps(Device* device, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	void TransitionImageLayout(Device* device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice physicalDevice);
	void EndCommandBuffer(Device* device, VkCommandBuffer commandBuffer, VkCommandPool pool, QueueFlags flag);

	void SetImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkImageSubresourceRange subresourceRange,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	void SetImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageAspectFlags aspectMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	void InsertImageMemoryBarrier(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		VkImageSubresourceRange subresourceRange);

	void CreateImage(Device* device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void CreateCubeMapImage(Device* device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkSampler CreateImageSampler(Device* device, float maxAnisotropy, float maxlod, VkSampler& sampler);
	VkImageView CreateImageView(Device* device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
}
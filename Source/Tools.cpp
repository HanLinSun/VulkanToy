#include "Tools.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <vulkan/Initializer.hpp>

void Tools::SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
	VkImageSubresourceRange subresourceRange,
	VkPipelineStageFlags srcStageMask,VkPipelineStageFlags dstStageMask)
{
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.subresourceRange.levelCount = 1;

	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		//Image Layout undefined
		imageMemoryBarrier.srcAccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		// Make sure host writes have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image is a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image is read by a shader
		// Make sure any shader reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}
	// Target layouts (new)
	switch (newImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image will be used as a transfer source
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image will be used as a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image layout will be used as a depth/stencil attachment
		// Make sure any writes to depth/stencil buffer have been finished
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image will be read in a shader (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if (imageMemoryBarrier.srcAccessMask == 0)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}

	vkCmdPipelineBarrier(
		cmdbuffer,
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);

}

void Tools::SetImageLayout(VkCommandBuffer cmdbuffer,VkImage image,	VkImageAspectFlags aspectMask,VkImageLayout oldImageLayout,VkImageLayout newImageLayout,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask)
{
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = aspectMask;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;
	SetImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
}

void Tools::InsertImageMemoryBarrier(
	VkCommandBuffer cmdbuffer,VkImage image,VkAccessFlags srcAccessMask,
	VkAccessFlags dstAccessMask,VkImageLayout oldImageLayout,VkImageLayout newImageLayout,
	VkPipelineStageFlags srcStageMask,VkPipelineStageFlags dstStageMask,
	VkImageSubresourceRange subresourceRange)
{
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.srcAccessMask = srcAccessMask;
	imageMemoryBarrier.dstAccessMask = dstAccessMask;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(
		cmdbuffer,
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);
}

VkCommandBuffer Tools::CreateCommandBuffer(Device* device, VkCommandBufferLevel level, VkCommandPool pool, bool begin)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = level;
	allocInfo.commandPool = pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device->GetVkDevice(), &allocInfo, &commandBuffer);

	if(begin)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
	}
	return commandBuffer;
}

void Tools::CreateCommandPool(Device* device, QueueFlags flag, VkCommandPoolCreateFlags createFlags, VkCommandPool* cmdPool)
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = device->GetInstance()->GetQueueFamilyIndices()[flag];
	cmdPoolInfo.flags = createFlags;

	if (vkCreateCommandPool(device->GetVkDevice(), &cmdPoolInfo, nullptr, cmdPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}

	std::cout<<"Dbg commandpool check: "<<*cmdPool<<std::endl;
}

void Tools::EndCommandBuffer(Device* device, VkCommandBuffer commandBuffer,VkCommandPool pool, QueueFlags flag)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(device->GetQueue(flag), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device->GetQueue(flag));
	// Wait for the fence to signal that command buffer has finished executing

	vkFreeCommandBuffers(device->GetVkDevice(), pool, 1, &commandBuffer);
}

void Tools::CopyBufferToImage(Device* device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = Tools::CreateCommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, device->GetGraphicCommandPool(), true);

	    VkBufferImageCopy region{};
	    region.bufferOffset = 0;
	    region.bufferRowLength = 0;
	    region.bufferImageHeight = 0;
	    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	    region.imageSubresource.mipLevel = 0;
	    region.imageSubresource.baseArrayLayer = 0;
	    region.imageSubresource.layerCount = 1;
	    region.imageOffset = { 0, 0, 0 };
	    region.imageExtent = {
	        width,
	        height,
	        1
	    };

	    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		Tools::EndCommandBuffer(device, commandBuffer, device->GetGraphicCommandPool(), QueueFlags::Graphics);
	}

void Tools::GenerateMipmaps(Device* device, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
       // Check if image format supports linear blitting
       VkFormatProperties formatProperties;
       vkGetPhysicalDeviceFormatProperties(device->GetInstance()->GetPhysicalDevice(), imageFormat, &formatProperties);

       if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
           throw std::runtime_error("texture image format does not support linear blitting!");
       }
	   VkCommandBuffer commandBuffer = Tools::CreateCommandBuffer(device,VK_COMMAND_BUFFER_LEVEL_PRIMARY, device->GetGraphicCommandPool(), true);

       VkImageMemoryBarrier barrier{};
       barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
       barrier.image = image;
       barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
       barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
       barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
       barrier.subresourceRange.baseArrayLayer = 0;
       barrier.subresourceRange.layerCount = 1;
       barrier.subresourceRange.levelCount = 1;

       int32_t mipWidth = texWidth;
       int32_t mipHeight = texHeight;

       for (uint32_t i = 1; i < mipLevels; i++) {
           barrier.subresourceRange.baseMipLevel = i - 1;
           barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
           barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
           barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
           barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

           vkCmdPipelineBarrier(commandBuffer,
               VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
               0, nullptr,
               0, nullptr,
               1, &barrier);

           VkImageBlit blit{};
           blit.srcOffsets[0] = { 0, 0, 0 };
           blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
           blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
           blit.srcSubresource.mipLevel = i - 1;
           blit.srcSubresource.baseArrayLayer = 0;
           blit.srcSubresource.layerCount = 1;
           blit.dstOffsets[0] = { 0, 0, 0 };
           blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
           blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
           blit.dstSubresource.mipLevel = i;
           blit.dstSubresource.baseArrayLayer = 0;
           blit.dstSubresource.layerCount = 1;

           vkCmdBlitImage(commandBuffer,
               image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
               image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
               1, &blit,
               VK_FILTER_LINEAR);

           barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
           barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
           barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
           barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

           vkCmdPipelineBarrier(commandBuffer,
               VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
               0, nullptr,
               0, nullptr,
               1, &barrier);

           if (mipWidth > 1) mipWidth /= 2;
           if (mipHeight > 1) mipHeight /= 2;
       }

       barrier.subresourceRange.baseMipLevel = mipLevels - 1;
       barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
       barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
       barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
       barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

       vkCmdPipelineBarrier(commandBuffer,
           VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
           0, nullptr,
           0, nullptr,
           1, &barrier);

	   Tools::EndCommandBuffer(device, commandBuffer, device->GetGraphicCommandPool(), QueueFlags::Graphics);
   }

void Tools::TransitionImageLayout(Device* device, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, VkPipelineStageFlags srcStageMask,VkPipelineStageFlags dstStageMask) {
	VkCommandBuffer commandBuffer = Tools::CreateCommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, device->GetGraphicCommandPool(), true);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	switch (oldLayout)
	{
	    case VK_IMAGE_LAYOUT_UNDEFINED:
		     barrier.srcAccessMask = 0;
		break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled 
			break;
	}

	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image layout will be used as a depth/stencil attachment
		// Make sure any writes to depth/stencil buffer have been finished
		barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image will be read in a shader (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if (barrier.srcAccessMask == 0)
		{
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		srcStageMask, dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	Tools::EndCommandBuffer(device, commandBuffer, device->GetGraphicCommandPool(), QueueFlags::Graphics);
}

VkSampleCountFlagBits Tools::GetMaxUsableSampleCount(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

void Tools::CreateImage(Device* device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = numSamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device->GetVkDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");

	}
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device->GetVkDevice(), image, &memRequirements);
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = device->GetInstance()->GetMemoryTypeIndex(memRequirements.memoryTypeBits, properties);
	if (vkAllocateMemory(device->GetVkDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}
	check_vk_result(vkBindImageMemory(device->GetVkDevice(), image, imageMemory, 0));
}

void Tools::CreateCubeMapImage(Device* device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = numSamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//By default each cubemap have only one layer
	imageInfo.arrayLayers = 6;
	imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	if (vkCreateImage(device->GetVkDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");

	}
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device->GetVkDevice(), image, &memRequirements);
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = device->GetInstance()->GetMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(device->GetVkDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}
	vkBindImageMemory(device->GetVkDevice(), image, imageMemory, 0);
}
VkImageView Tools::CreateImageView(Device* device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	// Specify color channel mappings (can be used for swizzling)
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device->GetVkDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}
	return imageView;
}

VkSampler Tools::CreateImageSampler(Device* device, float maxAnisotropy,float maxlod,VkSampler& sampler)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy =maxAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = maxlod;
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(device->GetVkDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

VkShaderModule Tools::CreateShaderModule(Device* device,const char *fileName)
{
	std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);
	if (is.is_open())
	{
		size_t size = is.tellg();
		is.seekg(0, std::ios::beg);
		char* shaderCode = new char[size];
		is.read(shaderCode, size);
		is.close();
		assert(size > 0);

		VkShaderModule shaderModule;
		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = size;
		moduleCreateInfo.pCode = (uint32_t*)shaderCode;

		check_vk_result(vkCreateShaderModule(device->GetVkDevice(), &moduleCreateInfo, NULL, &shaderModule));
		delete[] shaderCode;
		return shaderModule;
	}
	else
	{
		std::cerr << "Error: Could not open shader file \"" << fileName << "\"" << "\n";
		return VK_NULL_HANDLE;
	}
}

VkPipelineShaderStageCreateInfo Tools::LoadShader(Device* device, std::string fileName, VkShaderStageFlagBits stage,VkShaderModule& shaderModule)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.stage = stage;
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderModule = CreateShaderModule(device, fileName.c_str());
	shaderStage.module = shaderModule;
	shaderStage.pName = "main";
	assert(shaderStage.module != VK_NULL_HANDLE);
	return shaderStage;
}
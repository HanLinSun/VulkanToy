#include "Tools.h"

void Tools::SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
	VkImageSubresourceRange subresourceRange,
	VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
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

VkCommandPool Tools::CreateCommandPool(Device* device, QueueFlags flag, VkCommandPoolCreateFlags createFlags)
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = device->GetInstance()->GetQueueFamilyIndices()[flag];
	cmdPoolInfo.flags = createFlags;
	VkCommandPool cmdPool;
	if (vkCreateCommandPool(device->GetVkDevice(), &cmdPoolInfo, nullptr, &cmdPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics command pool!");
	}
	return cmdPool;
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

	vkFreeCommandBuffers(device->GetVkDevice(), pool, 1, &commandBuffer);
}

void Tools::CopyBufferToImage(Device* device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = Tools::CreateCommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, device->GetCommandPool(), true);

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

		Tools::EndCommandBuffer(device, commandBuffer, device->GetCommandPool(), QueueFlags::Graphics);
	}

void Tools::GenerateMipmaps(Device* device, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
       // Check if image format supports linear blitting
       VkFormatProperties formatProperties;
       vkGetPhysicalDeviceFormatProperties(device->GetInstance()->GetPhysicalDevice(), imageFormat, &formatProperties);

       if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
           throw std::runtime_error("texture image format does not support linear blitting!");
       }
	   VkCommandBuffer commandBuffer = Tools::CreateCommandBuffer(device,VK_COMMAND_BUFFER_LEVEL_PRIMARY, device->GetCommandPool(), true);

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

	   Tools::EndCommandBuffer(device, commandBuffer, device->GetCommandPool(), QueueFlags::Graphics);
   }

void Tools::TransitionImageLayout(Device* device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
	VkCommandBuffer commandBuffer = Tools::CreateCommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, device->GetCommandPool(), true);

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

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	Tools::EndCommandBuffer(device, commandBuffer, device->GetCommandPool(), QueueFlags::Graphics);
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
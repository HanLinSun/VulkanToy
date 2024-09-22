#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Device.h"

class FrameBuffer
{
public:
	FrameBuffer(Device* device, int numImageViews);

	void DestroyFrameBuffer();

protected:
	Device* m_device;
	std::vector<VkImageView> m_imageView;
	

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
};
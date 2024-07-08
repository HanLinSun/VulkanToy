#pragma once

//Vulkan Buffer class
#include <vector>
#include "VulkanTool.h"
#include <vulkan/vulkan.h>

namespace Renderer
{
	struct Buffer
	{
		VkDevice m_device;
		VkBuffer m_buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo m_descriptor;
		VkDeviceSize m_size = 0;
		VkDeviceSize m_alignment = 0;

		//pointer to mapped memory address
		void* m_mapped = nullptr;

		/** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
		VkBufferUsageFlags m_usageFlags;
		VkMemoryPropertyFlags m_memoryPropertyFlags;

		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void unmap();
		VkResult bind(VkDeviceSize offset = 0);
		void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void copyTo(void* data, VkDeviceSize size);
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void destroy();
	};

}
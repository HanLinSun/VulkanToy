#pragma once
#include <vulkan/vulkan.h>
#include <Vulkan/Device.h>
#include <Vulkan/Buffer.h>

namespace BufferUtils
{
	void CreateBuffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	VkResult CreateBuffer(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Buffer* buffer, VkDeviceSize size, void* data= nullptr);
	void CopyBuffer(Device* device, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CreateBufferFromData(Device* device, VkCommandPool commandPool, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	template <typename T>
	void CreateGPUBuffer(Device* device, const T* elements, int numSizes, VkBufferUsageFlags usage, Buffer* targetGPUBuffer);

	void CreateGPUTextureBuffer();
}
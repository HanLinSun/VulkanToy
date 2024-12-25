#pragma once
#include <vulkan/vulkan.h>
#include <Vulkan/Device.h>
#include <Vulkan/Buffer.h>
#include <Tools.h>

namespace BufferUtils
{
	void CreateBuffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	VkResult CreateBuffer(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Buffer* buffer, VkDeviceSize size, void* data= nullptr);
	void CopyBuffer(Device* device, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CreateBufferFromData(Device* device, VkCommandPool commandPool, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
   
    template <typename T>
    void CreateGPUBuffer(Device* device, T* elements, int numSizes, VkBufferUsageFlags usage, Buffer* targetGPUBuffer)
    {
        VkDeviceSize storagebufferSize = numSizes * sizeof(T);
        Buffer stagingBuffer;

        CreateBuffer(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, storagebufferSize, elements);
        CreateBuffer(device, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, targetGPUBuffer, storagebufferSize);

        VkCommandBuffer copyCmd = Tools::CreateCommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY, device->GetGraphicCommandPool(), true);
        VkBufferCopy copyRegion = { 0, 0, storagebufferSize };
        vkCmdCopyBuffer(copyCmd, stagingBuffer.buffer, targetGPUBuffer->buffer, 1, &copyRegion);

        Tools::EndCommandBuffer(device, copyCmd, device->GetGraphicCommandPool(), QueueFlags::Graphics);
        stagingBuffer.Destroy();
    }

}
#include "Headers/Vulkan/BufferUtils.h"
#include "Headers/Vulkan/Instance.h"
#include "Headers/Vulkan/Initializer.hpp"
#include <Tools.h>

void BufferUtils::CreateBuffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, 
	VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	//Create Buffer
    VkBufferCreateInfo bufferInfo = VulkanInitializer::bufferCreateInfo(usage, size);
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    check_vk_result(vkCreateBuffer(device->GetVkDevice(), &bufferInfo, nullptr, &buffer));
	
    // Query buffer's memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device->GetVkDevice(), buffer, &memRequirements);

    // Allocate memory in device
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device->GetInstance()->GetMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        allocInfo.pNext = &allocFlagsInfo;
    }
    check_vk_result(vkAllocateMemory(device->GetVkDevice(), &allocInfo, nullptr, &bufferMemory));
    // Associate allocated memory with vertex buffer
    vkBindBufferMemory(device->GetVkDevice(), buffer, bufferMemory, 0);
}

VkResult BufferUtils::CreateBuffer(Device* device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags properties, Buffer* buffer, VkDeviceSize size, void* data)
{
    buffer->device = device;

    VkBufferCreateInfo bufferCreateInfo = VulkanInitializer::bufferCreateInfo(usageFlags, size);
    check_vk_result(vkCreateBuffer(device->GetVkDevice(), &bufferCreateInfo, nullptr, &buffer->buffer));

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc = VulkanInitializer::memoryAllocateInfo();
    vkGetBufferMemoryRequirements(device->GetVkDevice(), buffer->buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = device->GetInstance()->GetMemoryTypeIndex(memReqs.memoryTypeBits, properties);
    // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    check_vk_result(vkAllocateMemory(device->GetVkDevice(), &memAlloc, nullptr, &buffer->memory));

    buffer->alignment = memReqs.alignment;
    buffer->size = size;
    buffer->usageFlags = usageFlags;
    buffer->memoryPropertyFlags = properties;

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr)
    {
        check_vk_result(buffer->Map());
        memcpy(buffer->mapped, data, size);
        if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            buffer->Flush();

        buffer->Unmap();
    }

    // Initialize a default descriptor that covers the whole buffer size
    buffer->SetupDescriptor();

    // Attach the memory to the buffer object
    return buffer->Bind();
}

void BufferUtils::CopyBuffer(Device* device, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device->GetVkDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device->GetQueue(QueueFlags::Graphics), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->GetQueue(QueueFlags::Graphics));
    vkFreeCommandBuffers(device->GetVkDevice(), commandPool, 1, &commandBuffer);
}

void BufferUtils::CreateBufferFromData(Device* device, VkCommandPool commandPool, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    // Create the staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkBufferUsageFlags stagingUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags stagingProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    BufferUtils::CreateBuffer(device, bufferSize, stagingUsage, stagingProperties, stagingBuffer, stagingBufferMemory);

    // Fill the staging buffer
    void* data;
    vkMapMemory(device->GetVkDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, bufferData, static_cast<size_t>(bufferSize));
    vkUnmapMemory(device->GetVkDevice(), stagingBufferMemory);

    // Create the buffer
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsage;
    VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    BufferUtils::CreateBuffer(device, bufferSize, usage, flags, buffer, bufferMemory);

    // Copy data from staging to buffer
    BufferUtils::CopyBuffer(device, commandPool, stagingBuffer, buffer, bufferSize);

    // No need for the staging buffer anymore
    vkDestroyBuffer(device->GetVkDevice(), stagingBuffer, nullptr);
    vkFreeMemory(device->GetVkDevice(), stagingBufferMemory, nullptr);
}


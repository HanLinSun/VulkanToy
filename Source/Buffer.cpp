#include <Vulkan/Buffer.h>
#include <assert.h>
#include <stdlib.h>
#include <cstring>
VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
	return vkMapMemory(device->GetVkDevice(), memory, offset, size, 0, &mapped);
}

void Buffer::Unmap()
{
	if (mapped)
	{
		vkUnmapMemory(device->GetVkDevice(), memory);
		mapped = nullptr;
	}
}

VkResult Buffer::Bind(VkDeviceSize offset)
{
	return vkBindBufferMemory(device->GetVkDevice(), buffer, memory, offset);
}

void Buffer::SetupDescriptor(VkDeviceSize size, VkDeviceSize offset)
{
	descriptor.offset = offset;
	descriptor.buffer = buffer;
	descriptor.range = size;
}

void Buffer::CopyTo(void* data, VkDeviceSize size)
{
	assert(mapped);
	memcpy(mapped, data, size);
}

VkResult Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = memory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	return vkFlushMappedMemoryRanges(device->GetVkDevice(), 1, &mappedRange);
}

VkResult Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = memory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	return vkInvalidateMappedMemoryRanges(device->GetVkDevice(), 1, &mappedRange);
}

void Buffer::Destroy()
{
	if (buffer)
	{
		vkDestroyBuffer(device->GetVkDevice(), buffer, nullptr);
	}
	if (memory)
	{
		vkFreeMemory(device->GetVkDevice(), memory, nullptr);
	}
}
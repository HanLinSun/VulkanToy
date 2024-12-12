#include <Vulkan/Buffer.h>
#include <assert.h>
#include <stdlib.h>
#include <cstring>
VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
	//used for writing data to GPU memory, mapped is a pointer to GPU memory address
	return vkMapMemory(device->GetVkDevice(), memory, offset, size, 0, &mapped);
}

void Buffer::Unmap()
{
	if (mapped)
	{
		//Unmaps a previously mapped GPU memory region, making it inaccessible to the CPU.
		//You must call VkUnmapMemory to release the mapping when you are done accessing the memory.
		//The memory is no longer accessible to the CPU after it is unmapped.

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
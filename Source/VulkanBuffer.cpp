#include "Headers/Vulkan/VulkanBuffer.h"
#include <assert.h>
namespace Renderer
{

/**
* Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
*
* @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
* @param offset (Optional) Byte offset from beginning
*
* @return VkResult of the buffer mapping call
*/
	VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset)
	{
		return vkMapMemory(m_device, m_deviceMemory, 0, m_size, 0, &m_mapped);
	}

	void Buffer::unmap()
	{
		if (m_mapped)
		{
			vkUnmapMemory(m_device, m_deviceMemory);
			m_mapped = nullptr;
		}
	}


	/**
	* Attach the allocated memory block to the buffer
	*
	* @param offset (Optional) Byte offset (from the beginning) for the memory region to bind
	*
	* @return VkResult of the bindBufferMemory call
	*/

	VkResult Buffer::bind(VkDeviceSize offset)
	{
		return vkBindBufferMemory(m_device, m_buffer, m_deviceMemory, offset);
	}

/**
* Setup the default descriptor for this buffer
*
* @param size (Optional) Size of the memory range of the descriptor
* @param offset (Optional) Byte offset from beginning
*
*/
	void Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)
	{
		m_descriptor.offset = offset;
		m_descriptor.buffer = m_buffer;
		m_descriptor.range = size;
	}

/**
* Copies the specified data to the mapped buffer
* @param data Pointer to the data to copy
* @param size Size of the data to copy in machine units
*/
	void Buffer::copyTo(void* data, VkDeviceSize size)
	{
		assert(m_mapped);
		memcpy(m_mapped, data, size);
	}

	VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = m_deviceMemory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(m_device, 1, &mappedRange);
	}


/**
* Invalidate a memory range of the buffer to make it visible to the host
*
* @note Only required for non-coherent memory
*
* @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
* @param offset (Optional) Byte offset from beginning
*
* @return VkResult of the invalidate call
*/

	VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = m_deviceMemory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(m_device, 1, &mappedRange);
	}

	void Buffer::destroy()
	{
		if (m_buffer)
		{
			vkDestroyBuffer(m_device, m_buffer, nullptr);
		}
		if (m_deviceMemory)
		{
			vkFreeMemory(m_device, m_deviceMemory, nullptr);
		}
	}

}

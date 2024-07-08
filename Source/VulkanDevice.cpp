#include "Headers/Vulkan/VulkanDevice.h"
#include <stdexcept>

namespace Renderer
{
	VulkanDevice::VulkanDevice(VkPhysicalDevice physcialDevice)
	{
		assert(physcialDevice);
		this->m_physicalDevice = physcialDevice;

		//Store device features, limits and properties of the physical device for later use
		//Device properties also contain limits and sparse properties
		vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
		//Features should be checked by the examples before using them
		vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_features);
		//Memory properties are used regularly for creating all kinds of buffers
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_deviceMemoryProperties);

		//queue family properties
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
		assert(queueFamilyCount > 0);

		m_queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());

		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr);

		if (extCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extCount);
			if (vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
			{
				for (auto& ext : extensions)
				{
					m_supportedExtensions.push_back(ext.extensionName);
				}
			}
		}
	}

	VulkanDevice::~VulkanDevice()
	{
		if (m_commandPool)
		{
			vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
		}
		if (m_logicalDevice)
		{
			vkDestroyDevice(m_logicalDevice, nullptr);
		}
	}

	uint32_t VulkanDevice::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
	{
		for (uint32_t i = 0; i < m_deviceMemoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((m_deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					if (memTypeFound)
					{
						*memTypeFound = true;
					}
					return i;
				}
			}
			typeBits >>= 1;
		}

		if (memTypeFound)
		{
			*memTypeFound = false;
			return 0;
		}
		else
		{
			throw std::runtime_error("Could not find a matching memory type");
		}
		return 0;
	}
	

	/**
* Get the index of a queue family that supports the requested queue flags
* SRS - support VkQueueFlags parameter for requesting multiple flags vs. VkQueueFlagBits for a single flag only
*
* @param queueFlags Queue flags to find a queue family index for
*
* @return Index of the queue family index that matches the flags
*
* @throw Throws an exception if no queue family index could be found that supports the requested flags
*/
	uint32_t VulkanDevice::getQueueFamilyIndex(VkQueueFlags queueFlags) const
	{
		return 0;
	}

	VkResult VulkanDevice::createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, void* pNextGen, bool useSwapChain, VkQueueFlags requestedQueueTypes)
	{
		return VkResult();
	}

	VkResult VulkanDevice::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data)
	{
		return VkResult();
	}

	VkResult VulkanDevice::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Buffer* buffer, VkDeviceSize size, void* data)
	{
		return VkResult();
	}

	void VulkanDevice::copyBuffer(Buffer* src, Buffer* dst, VkQueue queue, VkBufferCopy* copyRegion)
	{
	}

	VkCommandPool VulkanDevice::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
	{
		return VkCommandPool();
	}

	VkCommandBuffer VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin)
	{
		return VkCommandBuffer();
	}

	VkCommandBuffer VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, bool begin)
	{
		return VkCommandBuffer();
	}

	void VulkanDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free)
	{
	}

	void VulkanDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
	{
	}

	bool VulkanDevice::extensionSupported(std::string extension)
	{
		return false;
	}

	VkFormat VulkanDevice::getSupportedDepthFormat(bool checkSamplingSupport)
	{
		return VkFormat();
	}

}

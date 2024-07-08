#include "Headers/Vulkan/VulkanSwapChain.h"
#include <algorithm>

void VulkanSwapChain::initSurface(GLFWwindow* window)
{
	if (glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}

	//After create surface need to add swapChain support
	// Get available queue family properties

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
	assert(queueFamilyCount >= 1);

	std::vector<VkQueueFamilyProperties> queueProps(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueProps.data());

	// Iterate over each queue to learn whether it supports presenting :
	// Find a queue with present support
	// Will be used to present the swap chain images to the windowing system

	std::vector<VkBool32> supportsPresent(queueFamilyCount);
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &supportsPresent[i]);
	}

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (graphicsQueueNodeIndex == INT_MAX)
			{
				graphicsQueueNodeIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE)
			{
				graphicsQueueNodeIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}
		if (presentQueueNodeIndex == UINT32_MAX)
		{
			for (uint32_t i = 0; i < queueFamilyCount; i++)
			{
				if (supportsPresent[i] == VK_TRUE)
				{
					presentQueueNodeIndex = i;
					break;
				}
			}
		}

		//if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
		//{
		//	vks::tools::exitFatal("Could not find a graphics and/or presenting queue!", -1);
		//}

		//if (graphicsQueueNodeIndex != presentQueueNodeIndex)
		//{
		//	vks::tools::exitFatal("Separate graphics and presenting queues are not supported yet!", -1);
		//}

		m_queueNodeIndex = graphicsQueueNodeIndex;
		uint32_t formatCount;

		vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, NULL);
		assert(formatCount > 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, surfaceFormats.data());

		// get a format that best suits our needs, so we try to get one from a set of preferred formats
		// Initialize the format to the first one returned by the implementation in case we can't find one of the preffered formats
		VkSurfaceFormatKHR selectedFormat = surfaceFormats[0];

		std::vector<VkFormat> preferredImageFormat =
		{
			VK_FORMAT_B8G8R8A8_UNORM,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_FORMAT_A8B8G8R8_UNORM_PACK32
		};

		for (auto& availableFormat : surfaceFormats)
		{
			if (std::find(preferredImageFormat.begin(), preferredImageFormat.end(),availableFormat)!=preferredImageFormat.end())
			{
				selectedFormat = availableFormat;
				break;
			}
		}
		m_colorFormat = selectedFormat.format;
		m_colorSpace = selectedFormat.colorSpace;
}

//May need to modify this part
void VulkanSwapChain::setContext(const VkInstance& instance, const VkPhysicalDevice& physicalDevice,const VkDevice& device)
{
	this->m_instance = instance;
	this->m_physicalDevice = physicalDevice;
	this->m_device = device;
}

void VulkanSwapChain::create(uint32_t* width, uint32_t* height, bool vSync, bool fullScreen)
{
	assert(m_device);
	assert(m_physicalDevice);
	assert(m_instance);
	// Store the current swap chain handle so we can use it later on to ease up recreation
	VkSwapchainKHR oldSwapChain = m_swapChain;

	//Get Physical device surface properties 
	VkSurfaceCapabilitiesKHR surfCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfCaps);

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, NULL);
	assert(presentModeCount > 0);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.data());

	VkExtent2D swapChainExtent = {};
	if (surfCaps.currentExtent.width == (uint32_t)-1)
	{
		//if undefined
		//size is set to the sizes of images requests
		swapChainExtent.width = *width;
		swapChainExtent.height = *height;
	}
	else
	{
		swapChainExtent = surfCaps.currentExtent;
		*width = surfCaps.currentExtent.width;
		*height = surfCaps.currentExtent.height;
	}

	// Select a present mode for the swapChain
	// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
	// This mode waits for the vertical blank ("v-sync")
	VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	if (!vSync)
	{
		// If v-sync is not requested, try to find a mailbox mode
        // It's the lowest latency non-tearing present mode available
		for (size_t i = 0; i < presentModeCount; i++)
		{
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				swapChainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}
	//number of images
	uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;

	if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
	{
		desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
	}

	//Find transformation of the image
	VkSurfaceTransformFlagsKHR preTransform;
	if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		//prefer non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfCaps.currentTransform;
	}

	//Find a supported composite alpha format
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// Simply select the first composite alpha format available
	std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};

	for (auto& compositeAlphaFlag : compositeAlphaFlags)
	{
		if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag)
		{
			compositeAlpha = compositeAlphaFlag;
			break;
		}
	}

	VkSwapchainCreateInfoKHR swapChainCI = {};
	swapChainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCI.surface = m_surface;
	swapChainCI.minImageCount = desiredNumberOfSwapchainImages;
	swapChainCI.imageFormat = m_colorFormat;
	swapChainCI.imageColorSpace = m_colorSpace;
	swapChainCI.imageExtent = { swapChainExtent.width,swapChainExtent.height };
	swapChainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapChainCI.imageArrayLayers = 1;
	swapChainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCI.queueFamilyIndexCount = 0;
	swapChainCI.presentMode = swapChainPresentMode;

	swapChainCI.oldSwapchain = oldSwapChain;
	swapChainCI.clipped = VK_TRUE;
	swapChainCI.compositeAlpha = compositeAlpha;
	// Enable transfer source on swap chain images if supported
	if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
	{
		swapChainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	// Enable transfer destination on swap chain images if supported
	if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		swapChainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	vkCreateSwapchainKHR(m_device, &swapChainCI, nullptr, &m_swapChain);

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapChain != VK_NULL_HANDLE)
	{
		for (uint32_t i = 0; i < m_imageCount; i++)
		{
			vkDestroyImageView(m_device, m_buffers[i].view, nullptr);
		}
		vkDestroySwapchainKHR(m_device, oldSwapChain, nullptr);
	}

	vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_imageCount, NULL);
	//Get SwapChain Images
	m_images.resize(m_imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_imageCount, m_images.data());
	
	// Get the swap chain buffers containing the image and imageview
	m_buffers.resize(m_imageCount);

	//Create swapChain image views
	for (uint32_t i = 0; i < m_imageCount; i++)
	{
		VkImageViewCreateInfo colorAttachmentView = {};
		colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorAttachmentView.pNext = NULL;
		colorAttachmentView.format = m_colorFormat;
		colorAttachmentView.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A,
		};

		colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorAttachmentView.subresourceRange.baseMipLevel = 0;
		colorAttachmentView.subresourceRange.levelCount = 1;
		colorAttachmentView.subresourceRange.baseArrayLayer = 0;
		colorAttachmentView.subresourceRange.layerCount = 1;
		colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorAttachmentView.flags = 0;

		m_buffers[i].image = m_images[i];
		colorAttachmentView.image = m_buffers[i].image;
		vkCreateImageView(m_device, &colorAttachmentView, nullptr, &m_buffers[i].view);
	}
}

VkResult VulkanSwapChain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex)
{
	return vkAcquireNextImageKHR(m_device,m_swapChain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, imageIndex);
}

VkResult VulkanSwapChain::queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
{
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapChain;
	presentInfo.pImageIndices = &imageIndex;

	// Check if a wait semaphore has been specified to wait for before presenting the image
	if (waitSemaphore != VK_NULL_HANDLE)
	{
		presentInfo.pWaitSemaphores = &waitSemaphore;
		presentInfo.waitSemaphoreCount = 1;
	}
	return vkQueuePresentKHR(queue, &presentInfo);
}

void VulkanSwapChain::cleanUp()
{
	if (m_swapChain != VK_NULL_HANDLE)
	{
		for (uint32_t i = 0; i < m_imageCount; i++)
		{
			vkDestroyImageView(m_device, m_buffers[i].view, nullptr);
		}
	}
	if (m_surface != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}
	m_surface = VK_NULL_HANDLE;
	m_swapChain = VK_NULL_HANDLE;
}

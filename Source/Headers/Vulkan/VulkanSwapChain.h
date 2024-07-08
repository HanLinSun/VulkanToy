#pragma once
#include "../Source/Headers/RendererInclude.h"
#include "VulkanTool.h"
#include  <stdlib.h>
#include <string>
#include <assert.h>
#include <vulkan/vulkan.h>

typedef struct SwapChainBuffer
{
	VkImage image;
	VkImageView view;
};

//Vulkan SwapChain Submodule
class VulkanSwapChain
{
private:
	VkInstance m_instance;
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkSurfaceKHR m_surface;

public:
	VkFormat m_colorFormat;
	VkColorSpaceKHR m_colorSpace;
	VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
	uint32_t m_imageCount;
	std::vector<VkImage> m_images;
	std::vector<SwapChainBuffer> m_buffers;
	uint32_t m_queueNodeIndex = UINT32_MAX;

	void initSurface(GLFWwindow* window);

	/* Set the Vulkan objects required for swapchain creation and management, must be called before swapchain creation */
	void setContext(const VkInstance& instance, const VkPhysicalDevice& physicalDevice,const VkDevice& device);
	void create(uint32_t* width, uint32_t* height, bool vSync = false, bool fullScreen = false);
	VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
	VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

	void cleanUp();

};
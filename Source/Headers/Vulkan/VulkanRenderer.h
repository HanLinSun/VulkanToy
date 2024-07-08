#pragma once
#include <vulkan/vulkan.h>

namespace Renderer
{
	class VulkanBaseRenderer
	{
	public:
		VulkanBaseRenderer();
		virtual ~VulkanBaseRenderer();

		bool initVulkan();
		void createRenderer();
		void nextFrame();
		void updateOverlay();
		void createPipelineCache();
		void createCommandPool();
		void createSynchronizationPrimitives();
		void initSwapchain();
		void setupSwapChain();
		void createCommandBuffers();
		void destroyCommandBuffers();
	private:

		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;


	};

}

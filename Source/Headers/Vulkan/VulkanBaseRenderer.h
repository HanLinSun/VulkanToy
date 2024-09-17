#pragma once
#include <RendererInclude.h>
#include "Scene.h"
#include <cstdlib>
#include <imgui.h>
#include <Window.h>
#include "Instance.h"
#include "Device.h"
#include "QueueFlags.h"

const uint32_t WIDTH = 1600;
const uint32_t HEIGHT = 720;

const std::string MODEL_PATH = "./Models/Sponza/sponza.obj";
const std::string TEXTURE_PATH = "./Models/Sponza/textures";

//const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <Volk/volk.h>
#endif

#include <backends/imgui_impl_vulkan.h>
#include <LayerStack.h>
#include <ImGuiLayer.h>

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

namespace Renderer
{
	class VulkanBaseRenderer
	{
	public:
		VulkanBaseRenderer(Window* targetWindow);
		void Run();
		void Destroy();
		void Cleanup();
		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

		void SetLayerStack(LayerStack* layer);
		void InitGUILayerAttribute();
		void InitVulkan();
	private:

		Device* m_device;
		Instance* m_instance;
		GLFWwindow* m_window;
		SwapChain* m_swapChain;

		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		VkQueue m_presentQueue;
		//VkSwapchainKHR m_swapChain;

		std::vector<VkImage> m_swapChainImages;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		std::vector<VkImageView> m_swapChainImageViews;
		std::vector<VkFramebuffer> m_swapChainFramebuffers;

		VkRenderPass m_renderPass;
		VkRenderPass m_imGuiRenderPass;

		VkDescriptorSetLayout m_descriptorSetLayout;
		VkPipelineLayout m_pipelineLayout;

		VkPipeline m_graphicsPipeline;
		//RayTrace compute pipeline
		VkPipeline m_computePipeline;

		VkCommandPool m_commandPool;
		VkImage m_colorImage;
		VkDeviceMemory m_colorImageMemory;
		VkImageView m_colorImageView;

		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;

		uint32_t mipLevels;

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;


		VkDescriptorPool m_descriptorPool;
		VkDescriptorSet m_cameraDescriptorSet;

		std::vector<VkDescriptorSet> m_modelDescriptorSets;
		std::vector<VkDescriptorSet> m_computeDescriptorSets;

		VkDescriptorPool m_imGuiDescriptorPool;
		std::vector<VkDescriptorSet> m_imGuiDescriptorSet;

		std::vector<VkCommandBuffer> m_commandBuffers;
		std::vector<VkCommandBuffer> m_imGuiCommandBuffers;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		uint32_t currentFrame = 0;

		Scene* m_Scene;
		Camera m_Camera;
		bool show_demo_window = true;
		QueueFamilyIndices queueFamilyIndices;
		uint32_t imageCount;

		ImDrawData* draw_data = nullptr;
		LayerStack* m_layerStack;
		ImGuiLayer* m_ImGuiLayer;

		int ScreenWidth;
		int ScreenHeight;

		//update time
		std::chrono::time_point<std::chrono::high_resolution_clock> lastTimeStamp, tPrevEnd;

		bool framebufferResized = false;


		void CreateScene();

		void LoadModel(std::string model_path, std::string model_texture_path);

		void CompileShader(std::string vertexShader, std::string fragmentShader);

		void DestroyFrameResources();

		void RecreateSwapChain();

		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void SetupDebugMessenger();

		void CreateSurface();

		void CreateImageViews();

		void CreateRenderPass();
		//GUI Pass
		void CreateDescriptorSetLayout();

		void CreateGraphicsPipeline();

		void CreateComputePipeline();

		void CreateFramebuffers();

		void CreateCommandPool(VkCommandPool* commandPool);

		void CreateColorResources();

		void CreateDepthResources();

		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		VkFormat FindDepthFormat();

		bool HasStencilComponent(VkFormat format);

		void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void CreateDescriptorPool();

		void CreateModelDescriptorSets(int shaderBindings);

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		VkCommandBuffer BeginSingleTimeCommands();

		void EndSingleTimeGraphicCommands(VkCommandBuffer commandBuffer);

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void createCommandBuffers();

		void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		void createSyncObjects();

		void updateUniformBuffer(uint32_t currentImage);

		void drawFrame();

		VkShaderModule createShaderModule(const std::vector<char>& code);

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

		bool checkDeviceExtensionSupport(VkPhysicalDevice device);

		std::vector<const char*> getRequiredExtensions();

		bool checkValidationLayerSupport();

		static std::vector<char> readFile(const std::string& filename);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		}
	};

}
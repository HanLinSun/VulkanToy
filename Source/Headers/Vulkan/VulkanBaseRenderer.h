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
const std::string MODEL_FILE_PATH = "./Models/Sponza/";

//const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	//Device Extensions
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
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

		std::vector<VkImageView> m_imageViews;
		std::vector<VkFramebuffer> m_framebuffers;

		VkRenderPass m_renderPass;
		VkRenderPass m_imGuiRenderPass;

		VkDescriptorSetLayout m_cameraDescriptorSetLayout;
		VkDescriptorSetLayout m_modelDescriptorSetLayout;


		VkPipelineLayout m_graphicPipelineLayout;

		VkPipeline m_graphicsPipeline;
		//RayTrace compute pipeline
		VkPipeline m_computePipeline;

		VkCommandPool m_commandPool;

		VkImage m_msaaCoIorImage;
		VkDeviceMemory m_msaaColorImageMemory;
		VkImageView m_msaaColorImageView;

		VkImage m_depthImage;
		VkDeviceMemory m_depthImageMemory;
		VkImageView m_depthImageView;

		uint32_t mipLevels;

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;


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
		Camera* m_Camera;
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

		void CreateRenderPass();
		//GUI Pass
		void CreateCameraDescriptorSetLayout();

		void CreateModelDescriptorSetLayout();

		void CreateGraphicsPipeline();

		void CreateComputePipeline();

		void CreateFrameResources();

		void CreateCommandPool(VkCommandPool* commandPool);

		bool HasStencilComponent(VkFormat format);

		void CreateDescriptorPool();

		void RecreateFrameResources();

		void CreateCameraDescriptorSets();

		void CreateModelDescriptorSets(int shaderBindings);

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		void CreateUniformBuffer();

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void CreateCommandBuffers();

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		void CreateSyncObjects();

		void UpdateUniformBuffer(uint32_t currentImage);

		void DrawFrame();

		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

		std::vector<const char*> GetRequiredExtensions();

		bool CheckValidationLayerSupport();

		static std::vector<char> readFile(const std::string& filename);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		}
	};

}
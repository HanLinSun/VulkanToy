#pragma once
#include <RendererInclude.h>
#include "Scene.h"
#include <cstdlib>
#include <imgui.h>
#include <Window.h>
#include "Instance.h"
#include <Vulkan/Device.h>
#include <Vulkan/QueueFlags.h>
#include <Input.h>
#include <Timestep.h>
#include <ThreadPool.hpp>
#include <Event/KeyEvent.h>
#include <Event/MouseEvent.h>
#include <Vulkan/Initializer.hpp>
#include <Render/RayTraceModule.h>

const uint32_t WIDTH = 1600;
const uint32_t HEIGHT = 720;

const std::string SCENE_FILE_PATH = "./Scene/cornellbox_test.txt";
//Texture folder need to put in model file path
const std::string MODEL_PATH = "./Models/CornellBox/CornellBox-Original.obj";
const std::string MODEL_FILE_PATH = "./Models/CornellBox/";

//Seems that this object is too large and will crash vulkan driver��solved��
//const std::string MODEL_PATH = "./Models/Sponza/sponza.obj";
//const std::string MODEL_FILE_PATH = "./Models/Sponza/";
const std::string SKYBOX_CUBEMAP_PATH = "./Textures/Skybox/";

const std::vector<std::string> cubeMapPaths
{
	    SKYBOX_CUBEMAP_PATH + "posx.jpg",
		SKYBOX_CUBEMAP_PATH + "negx.jpg",
		SKYBOX_CUBEMAP_PATH + "posy.jpg",
		SKYBOX_CUBEMAP_PATH + "negy.jpg",
		SKYBOX_CUBEMAP_PATH + "posz.jpg",
		SKYBOX_CUBEMAP_PATH + "negz.jpg"
};

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

struct ThreadPushConstantBlock
{
	glm::mat4 mvp;
    glm::vec3 color;
};

struct ThreadData
{
	VkCommandPool commandPool { VK_NULL_HANDLE };
	// One command buffer per render object
	std::vector<VkCommandBuffer> commandBuffers;
	// One push constant block per render object
	std::vector<ThreadPushConstantBlock> pushConstantBlocks;
	// Per object information (position, rotation, etc.)
	//std::vector<ObjectData> objectData;
};


namespace Renderer
{
	class Engine
	{
	public:
		Engine(Window* targetWindow);
		void Run();
		void Destroy();
		void Cleanup();
		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

		void OnEvent(Event& e);
		void SetLayerStack(LayerStack* layer);
		void InitGUILayerAttribute();
		void InitVulkan();

		VkSampler default_sampler = {};

		std::vector<VkSampler> temp_samplers;
	private:

		std::unique_ptr<RayTraceModule> m_RayTraceModule;

		std::shared_ptr<Device> m_device;
		std::unique_ptr<Instance> m_instance;
		GLFWwindow* m_window;
		std::shared_ptr<SwapChain> m_swapChain;

		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkSurfaceKHR m_surface;
		VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		VkQueue m_presentQueue;
				
		//VkSwapchainKHR m_swapChain;

		std::vector<VkImageView> m_imageViews;
		std::vector<VkFramebuffer> m_framebuffers;

		VkRenderPass m_renderPass;

		VkDescriptorSetLayout m_cameraDescriptorSetLayout;
		VkDescriptorSetLayout m_materialDescriptorLayout;

		VkPipelineLayout m_graphicPipelineLayout;

		VkPipeline m_graphicsPipeline;
		VkPipelineCache m_pipelineCache{ VK_NULL_HANDLE };

		//This new image will have to store the desired number of samples per pixel
		VkImage m_colorImage;
		VkDeviceMemory m_colorImageMemory;
		VkImageView m_colorImageView;

		VkImage m_depthImage;
		VkDeviceMemory m_depthImageMemory;
		VkImageView m_depthImageView;

		uint32_t mipLevels;
		VkSubmitInfo m_submitInfo;

		//After finish raytrace pipeline need to fix this
		VkDescriptorPool m_descriptorPool;

		VkDescriptorSet m_cameraDescriptorSet;

		//For raytrace pipeline, only need one image
		VkDescriptorSetLayout m_rayTraceGraphicsDescriptorLayout;
		VkDescriptorSet m_rayTraceGraphicsDescriptorSet;
		ComputeResource m_rayTraceResource;
		VkPipelineLayout m_rayTraceGraphicsPipelineLayout;
		VkPipeline m_rayTraceGraphicsPipeline;
		//

		std::vector<VkDescriptorSet> m_materialDescriptorSets;
	
		VkDescriptorPool m_raytraceGraphicsDescriptorPool;
		
		std::vector<VkCommandBuffer> m_commandBuffers;

		struct
		{
			VkSemaphore presentComplete;
			VkSemaphore renderComplete;
		}m_Semaphores;

		Texture m_storageImage;

		std::vector<VkFence> m_waitFences;

		// Active frame buffer index
		uint32_t currentFrame = 0;

		std::unique_ptr<Scene> m_Scene;
		std::shared_ptr<CameraController> m_CameraController;
		std::shared_ptr<Camera> m_Camera;

		std::vector<ThreadData> m_threadDatas;

		ThreadPool threadPool;
		std::shared_ptr<Timestep> m_time;

		bool show_demo_window = true;
		QueueFamilyIndices queueFamilyIndices;
		uint32_t imageCount;

		ImDrawData* draw_data = nullptr;
		LayerStack* m_layerStack;
		ImGuiLayer* m_ImGuiLayer;

		int ScreenWidth;
		int ScreenHeight;

		bool m_runRaytracePipeline;

		//update time
		std::chrono::time_point<std::chrono::high_resolution_clock> lastTimeStamp, tPrevEnd;

		bool framebufferResized = false;

		void LoadModel(std::string model_path, std::string model_texture_path);

		void LoadScene(std::string SceneFile);

		void DestroyFrameResources();

		void RecreateSwapChain();

		void CreateSubmitInfo();

		VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore,uint32_t *imagerIndex);

		VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice device);

		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void CreatePipelineCache();

		void SetupDebugMessenger();

		void CreateSurface(); 

		void CreateRenderPass();
		//GUI Pass
		void CreateCameraDescriptorSetLayout();

		void CreateModelDescriptorSetLayout();

		void CreateRayTraceGraphicsDescriptorPool();

		//This function creates graphics part of descriptor layout and descriptor sets
		void CreateRayTraceGraphicDescriptorResources();

		void CreateRayTraceGraphicsPipeline();

		void UpdateIOInput();

		void CreateGraphicsPipeline();

		void CreateFrameResources();

		void CreateGraphicsCommandPool(VkCommandPool* commandPool);

		bool HasStencilComponent(VkFormat format);

		void CreateDescriptorPool();

		void RecreateFrameResources();

		void CreateCameraDescriptorSets();

		void CreateModelDescriptorSets(int shaderBindings);

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void CreateCommandBuffers();

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		void RecordRayTraceGraphicCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		void CreateSyncObjects();

		void CreateSkyboxCubeMap(std::string cubeMap_texturePath);

		void DrawFrame();

		void LoadCubeMapTexture(std::vector<std::string>& texturePath );

	    void LoadCubeMapTexture();

		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		std::vector<const char*> GetRequiredExtensions();

		bool CheckValidationLayerSupport();

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		}
	};

}
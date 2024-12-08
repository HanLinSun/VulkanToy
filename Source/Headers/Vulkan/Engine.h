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

const uint32_t WIDTH = 1600;
const uint32_t HEIGHT = 720;

const std::string MODEL_PATH = "./Models/Sponza/sponza.obj";
const std::string MODEL_FILE_PATH = "./Models/Sponza/";
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

// Resources for the compute part of the example
struct ComputeResource {
	// Object properties for planes and spheres are passed via a shade storage buffer
	// There is no vertex data, the compute shader calculates the primitives on the fly
	Buffer objectStorageBuffer;
	Buffer uniformBuffer;										// Uniform buffer object containing scene parameters
	//VkQueue queue{ VK_NULL_HANDLE };								// Separate queue for compute commands (queue family may differ from the one used for graphics)
	VkCommandPool commandPool{ VK_NULL_HANDLE };					// Use a separate command pool (queue family may differ from the one used for graphics)
	VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };				// Command buffer storing the dispatch commands and barriers
	VkFence fence{ VK_NULL_HANDLE };								// Synchronization fence to avoid rewriting compute CB if still in use
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };	// Compute shader binding layout
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };				// Compute shader bindings
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };				// Layout of the compute pipeline
	VkPipeline pipeline{ VK_NULL_HANDLE };							// Compute raytracing pipeline
	struct UniformDataCompute {										   // Compute shader uniform block object
		glm::vec3 lightPos;
		float aspectRatio{ 1.0f };
		glm::vec4 fogColor = glm::vec4(0.0f);
		struct {
			glm::vec3 pos = glm::vec3(0.0f, 0.0f, 4.0f);
			glm::vec3 lookat = glm::vec3(0.0f, 0.5f, 0.0f);
			float fov = 10.0f;
		} camera;
		glm::mat4 _pad;
	} uniformData;
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


		std::vector<VkSampler> temp_samplers;
	private:

		std::shared_ptr<Device> m_device;
		std::unique_ptr<Instance> m_instance;
		GLFWwindow* m_window;
		SwapChain* m_swapChain;

		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkSurfaceKHR m_surface;
		VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		VkQueue m_presentQueue;
				
		//VkSwapchainKHR m_swapChain;

		std::vector<VkImageView> m_imageViews;
		std::vector<VkFramebuffer> m_framebuffers;

		VkRenderPass m_renderPass;

		VkDescriptorSetLayout m_cameraDescriptorSetLayout;
		VkDescriptorSetLayout m_modelDescriptorSetLayout;

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

		VkDescriptorPool m_descriptorPool;
		VkDescriptorSet m_cameraDescriptorSet;

		std::vector<VkDescriptorSet> m_modelDescriptorSets;
		std::vector<VkDescriptorSet> m_computeDescriptorSets;

		VkDescriptorPool m_imGuiDescriptorPool;
		std::vector<VkDescriptorSet> m_imGuiDescriptorSet;

		std::vector<VkCommandBuffer> m_commandBuffers;

		struct
		{
			VkSemaphore presentComplete;
			VkSemaphore renderComplete;
		}m_Semaphores;


		ComputeResource m_rayTraceResources;
		Texture m_storageImage;

		std::vector<VkFence> m_waitFences;

		// Active frame buffer index
		uint32_t currentFrame = 0;

		std::unique_ptr<Scene> m_Scene;
		std::shared_ptr<CameraController> m_CameraController;
		std::shared_ptr<Camera> m_Camera;
		std::unique_ptr<TextureCubeMap> m_skyboxTexture;

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

		void CompileShader(std::string vertexShader, std::string fragmentShader);

		void DestroyFrameResources();

		void RecreateSwapChain();

		void CreateSubmitInfo();

		VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore,uint32_t *imagerIndex);

		VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice device);

		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void CreateSynchronizationPrimitives();

		void CreatePipelineCache();

		void SetupDebugMessenger();

		void CreateSurface(); 

		void CreateRenderPass();
		//GUI Pass
		void CreateCameraDescriptorSetLayout();

		void CreateModelDescriptorSetLayout();

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

		void CreateSyncObjects();

		void CreateSkyboxCubeMap(std::string cubeMap_texturePath);

		//Raytrace Compute Pipeline
		void CreateRayTracePipeline();
		void CreateRayTraceCommandPool(VkCommandPool* commandPool);
		void CreateRayTraceDescriptorSet();
		void CreateRayTraceStorageImage();
		//
		void DrawFrame();

		void RecordComputeCommandBuffer();

		void LoadCubeMapTexture(std::vector<std::string>& texturePath );

	    void LoadCubeMapTexture();

		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		std::vector<const char*> GetRequiredExtensions();

		bool CheckValidationLayerSupport();

		static std::vector<char> readFile(const std::string& filename);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		}
	};

}
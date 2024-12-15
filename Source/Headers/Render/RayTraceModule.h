#pragma once
#include <Tools.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <Vulkan/Initializer.hpp>
#include <Vulkan/Buffer.h>
#include <Vulkan/Texture.h>
#include <Vulkan/BufferUtils.h>
#include <Vulkan/Buffer.h>
#include <Camera.h>
#include <Log.h>

namespace Renderer
{
	struct RayTraceUniformData
	{										// Compute shader uniform block object

		glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 4.0f);
		glm::vec3 cam_lookat = glm::vec3(0.0f, 0.5f, 0.0f);
		float fov = 10.0f;
		float aspectRatio{ 1.0f };
	};

	struct ComputeResource {
		// Object properties for planes and spheres are passed via a shade storage buffer
		// There is no vertex data, the compute shader calculates the primitives on the fly
		//Buffer objectStorageBuffer;
		Buffer uniformBuffer;										// Uniform buffer object containing scene parameters
		//VkQueue queue{ VK_NULL_HANDLE };								// Separate queue for compute commands (queue family may differ from the one used for graphics)
		VkCommandPool commandPool{ VK_NULL_HANDLE };					// Use a separate command pool (queue family may differ from the one used for graphics)
		VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };				// Command buffer storing the dispatch commands and barriers
		VkFence fence{ VK_NULL_HANDLE };								// Synchronization fence to avoid rewriting compute CB if still in use
		VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };	// Compute shader binding layout
		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };				// Compute shader bindings
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };				// Layout of the compute pipeline
		VkPipeline pipeline{ VK_NULL_HANDLE };							// Compute raytracing pipeline

		void DestroyVKResource();
	};

	class RayTraceModule
	{
	public:
		RayTraceModule(std::shared_ptr<Device> device);
		~RayTraceModule()=default;

		void DestroyVKResources();
		void RecordComputeCommandBuffer();
		void CreateRayTracePipeline();
		void CreateRayTraceDescriptorSet();
		void CreateDescriptorPool(VkDescriptorPool& m_descriptorPool);
		void CreateRayTraceCommandPool(VkCommandPool* commandPool);
		void CreateRayTraceStorageImage(uint32_t width, uint32_t height);
		void UpdateUniformBuffer(Camera* cam);
		void CreateUniformBuffer();
		ComputeResource GetRayTraceComputeResource() const;
		Texture2D GetStorageImage() const;

	private:
		ComputeResource m_rayTraceResources;
		RayTraceUniformData m_rayTraceUniform;

		std::shared_ptr<Device> m_device;

		VkDescriptorPool m_descriptorPool;
	
		Texture2D m_storageImage;

	};
}

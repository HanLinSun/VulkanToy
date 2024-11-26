#pragma once
#include <Layer.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <Core.h>
#include <Vulkan/Buffer.h>
#include <Vulkan/Device.h>
#include <Tools.h>
#include <imgui.h>

namespace Renderer
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer()=default;

		void OnDetach() override;
		void OnEvent(Event& event);

		void SetStyle(uint32_t index);
		//Vulkan needs them
		void CreateImGuiDescriptorPool(uint32_t maxSets);
		void InitImGUIAttribute(Device* device, VkExtent2D& in_swapChainExtent, VkRenderPass renderPass, VkQueue copyQueue, const std::string& shadersPath, VkSampleCountFlagBits msaaSample);

		void NewFrame();

		void InitVulkanResources(VkRenderPass renderPass, VkQueue copyQueue, const std::string& shadersPath, VkSampleCountFlagBits msaaSample);
		bool Update();

		void UpdateImGUIEvent();
		void DrawFrame(VkCommandBuffer commandBuffer);

		void Destroy();
		void Begin();
		void End();
		void BlockEvents(bool block) { m_blockEvents = block; }

		// UI params are set via push constants
		struct PushConstBlock {
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

		bool m_updated{ false };

		std::vector<VkCommandBuffer> m_imGuiCommandBuffers;
		std::vector<VkFramebuffer> m_imGuiFrameBuffers;
		VkRenderPass m_imGuiRenderPass;

		VkCommandPool m_commandPool;

	private:

		//Vulkan Resources for rendering UI (old method will lead to framebuffer issue)
		Device* m_device;
		// Vulkan resources for rendering the UI
		VkSampler m_sampler;
		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;

		int32_t m_vertexCount = 0;
		int32_t m_indexCount = 0;

		VkDeviceMemory m_fontMemory = VK_NULL_HANDLE;
		VkImage m_fontImage = VK_NULL_HANDLE;
		VkImageView m_fontView = VK_NULL_HANDLE;

		VkPipelineCache m_pipelineCache;
		VkPipelineLayout m_pipelineLayout;
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorSet m_descriptorSet;
		VkPhysicalDeviceDriverProperties m_driverProperties = {};
		VkPipeline m_pipeline;

		VkDescriptorPool m_descriptorPool;

		VkShaderModule m_vertexShaderModule;
		VkShaderModule m_fragmentShaderModule;

		ImGuiIO* io = nullptr;
		ImDrawData* m_drawData = nullptr;

		VkExtent2D m_swapChainExtent;
		ImGuiStyle vulkanStyle;

		bool m_blockEvents = false;
	};
}

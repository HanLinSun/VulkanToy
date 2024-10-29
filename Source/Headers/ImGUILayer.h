#pragma once
#include <Layer.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <Core.h>
#include <Vulkan/Buffer.h>
#include <Vulkan/Device.h>
#include <Tools.h>

namespace Renderer
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer()=default;

		void OnUpdate() override;
		void OnDetach() override;
		void OnAttach() override;
		void OnEvent(Event& event);

		void DrawUI(uint32_t currentFrame, uint32_t imageIndex);

		void SetStyle(uint32_t index);
		//Vulkan needs them
		void CreateImGuiDescriptorPool();
		void CreateImGuiRenderPass(VkFormat swapChainImageFormat);
		void CreateImGuiCommandBuffers();
		void CreateImGuiFramebuffer(std::vector<VkImageView>& swapChainImageViews);
		void InitImGUIAttribute(Device* device, VkExtent2D& swapChainExtent, VkFormat& swapChainImageFormat, std::vector<VkImageView>& swapChainImageViews, uint32_t width, uint32_t height);

		VkDescriptorPool GetDescriptorPool() const;
		VkCommandPool GetCommandPool() const;

		void InitVulkanResources();

		void Destroy();
		void Begin();
		void End();
		void BlockEvents(bool block) { m_blockEvents = block; }


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

		ImGuiIO* io = nullptr;
		ImDrawData* m_drawData = nullptr;

		VkExtent2D m_swapChainExtent;
		ImGuiStyle vulkanStyle;

		bool m_blockEvents = false;
	};
}

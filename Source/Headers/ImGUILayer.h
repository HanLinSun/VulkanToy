#pragma once
#include <Layer.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <Core.h>

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

		//Vulkan needs them
		void CreateImGuiDescriptorPool();
		void CreateImGuiRenderPass(VkFormat swapChainImageFormat);
		void CreateImGuiCommandBuffers();
		void CreateImGuiFramebuffer(std::vector<VkImageView>& swapChainImageViews);

		void InitImGUIAttribute(const VkDevice& in_device, VkExtent2D& swapChainExtent, VkFormat& swapChainImageFormat, std::vector<VkImageView>& swapChainImageViews, uint32_t width, uint32_t height);

		void Destroy();

		void Begin();
		void End();
		void BlockEvents(bool block) { m_blockEvents = block; }

		VkDescriptorPool m_imGuiDescriptorPool;
		std::vector<VkDescriptorSet> m_imGuiDescriptorSet;
		std::vector<VkCommandBuffer> m_imGuiCommandBuffers;
		std::vector<VkFramebuffer> m_imGuiFrameBuffers;
		VkRenderPass m_imGuiRenderPass;
		VkCommandPool m_imGuiCommandPool;

	private:

		ImGuiIO* io = nullptr;
		ImDrawData* m_drawData = nullptr;
		VkDevice m_device;
		VkExtent2D m_swapChainExtent;

		bool m_blockEvents = false;
	};
}

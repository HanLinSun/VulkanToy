#pragma once
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <shobjidl.h> 

const int MAX_FRAMES_IN_FLIGHT = 2;
class GUIManager
{
public:
	//GUIManager() {};
	~GUIManager() {};

    
	void createImGuiDescriptorPool();
	void createImGuiRenderPass( VkFormat swapChainImageFormat);
	void createImGuiCommandBuffers();
	void createImGuiFramebuffer(std::vector<VkImageView>& swapChainImageViews);

	void initImGUIAttribute(VkDevice in_device, VkExtent2D swapChainExtent, VkFormat swapChainImageFormat,std::vector<VkImageView>& swapChainImageViews, uint32_t width, uint32_t height);
	void createGUIFrame();

	void destroy();
	bool openFile(std::string filePath);
	void drawUI(uint32_t currentFrame, uint32_t imageIndex);

	VkDescriptorPool m_imGuiDescriptorPool;
	std::vector<VkDescriptorSet> m_imGuiDescriptorSet;
	std::vector<VkCommandBuffer> m_imGuiCommandBuffers;
	std::vector<VkFramebuffer> m_imGuiFrameBuffers;
	VkRenderPass m_imGuiRenderPass;
	VkCommandPool m_imGuiCommandPool;

private:
	ImDrawData* draw_data = nullptr;
	VkDevice device;
	VkExtent2D swapChainExtent;

	ImGuiIO* io=nullptr;

	std::string loadModelPath;
	std::string loadTexturePath;
};


#pragma once
#include <array>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include "QueueFlags.h"
#include "SwapChain.h"
#include <glfw/glfw3.h>

class SwapChain;
class Instance;
class Device
{

	using Queues = std::array<VkQueue, sizeof(QueueFlags)>;
	friend class Instance;
public:
	SwapChain* CreateSwapChain(VkSurfaceKHR surface, unsigned int numBuffers, GLFWwindow* window);
	Instance* GetInstance();
	VkDevice GetVkDevice();
	VkQueue GetQueue(QueueFlags flag);
	VkCommandPool GetGraphicCommandPool();
	unsigned int GetQueueIndex(QueueFlags flag);
	~Device();

	VkCommandPool m_graphicsCommandPool;
	VkCommandPool m_computeCommandPool;

	Device(Instance* instance, VkDevice vkDevice, Queues queues);

private:

	Device() = delete;

	Instance* m_instance;
	VkDevice m_vkDevice;
	Queues m_queues;
};
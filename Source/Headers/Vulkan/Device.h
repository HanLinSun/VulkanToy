#pragma once
#include <array>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include "QueueFlags.h"
#include "SwapChain.h"
#include <glfw/glfw3.h>
#include <memory>

class SwapChain;
class Instance;
class Device: public std::enable_shared_from_this<Device>
{

	using Queues = std::array<VkQueue, sizeof(QueueFlags)>;
	friend class Instance;
public:
	std::shared_ptr<SwapChain> CreateSwapChain(VkSurfaceKHR surface, unsigned int numBuffers, GLFWwindow* window);
	Instance* GetInstance();
	VkDevice GetVkDevice();
	VkQueue GetQueue(QueueFlags flag);
	VkCommandPool GetGraphicCommandPool();
	unsigned int GetQueueIndex(QueueFlags flag);
	~Device();

	VkCommandPool m_graphicsCommandPool;
	VkCommandPool m_computeCommandPool;

	Device(Instance* instance, VkDevice vkDevice, Queues queues);
	void DestroyVKResources();

private:

	Device() = delete;

	Instance* m_instance;
	VkDevice m_vkDevice;
	Queues m_queues;
};
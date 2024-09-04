#pragma once
#include <array>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include "Vulkan/QueueFlags.h"

class SwapChain;
class Instance;
class Device
{
	friend class Instance;
public:
	SwapChain* CreateSwapChain(VkSurfaceKHR surface, unsigned int numBuffers);
	Instance* GetInstance();
	VkDevice GetVkDevice();
	VkQueue GetQueue(QueueFlags flag);
	unsigned int GetQueueIndex(QueueFlags flag);
	~Device();

private:
	using Queues = std::array<VkQueue, sizeof(QueueFlags)>;

	Device() = delete;
	Device(Instance* instance, VkDevice vkDevice, Queues queues);

	Instance* m_instance;
	VkDevice m_vkDevice;
	Queues m_queues;
};
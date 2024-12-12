#include "Headers/Vulkan/Device.h"
#include "Headers/Vulkan/Instance.h"

Device::Device(Instance* instance, VkDevice vkDevice,Queues queues):m_instance(instance),m_vkDevice(vkDevice),m_queues(queues) {}
Instance* Device::GetInstance()
{
	return m_instance;
}

VkDevice Device::GetVkDevice()
{
	return m_vkDevice;
}

VkQueue Device::GetQueue(QueueFlags flag)
{
	return m_queues[flag];
}

unsigned int Device::GetQueueIndex(QueueFlags flag) {
	return GetInstance()->GetQueueFamilyIndices()[flag];
}

VkCommandPool Device::GetGraphicCommandPool()
{
	return m_graphicsCommandPool;
}

std::shared_ptr<SwapChain> Device::CreateSwapChain(VkSurfaceKHR surface, unsigned int numBuffers, GLFWwindow* window)
{
	return std::make_shared<SwapChain>(shared_from_this(), surface, numBuffers, window);
}

void Device::DestroyVKResources()
{
	vkDestroyCommandPool(m_vkDevice, m_graphicsCommandPool, nullptr);
	vkDestroyCommandPool(m_vkDevice, m_computeCommandPool, nullptr);
	vkDestroyDevice(m_vkDevice, nullptr);
}

Device::~Device() {}

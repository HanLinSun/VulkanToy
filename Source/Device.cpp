#include "Headers/Vulkan/Device.h"
#include "Headers/Vulkan/Instance.h"

Device::Device(Instance* instance, VkDevice vkDevice, Queues queues):m_instance(instance),m_vkDevice(vkDevice),m_queues(queues){}
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

SwapChain* Device::CreateSwapChain(VkSurfaceKHR surface, unsigned int numBuffers, GLFWwindow* window)
{
	return new SwapChain(this, surface, numBuffers, window);
}

Device::~Device() {
	vkDestroyDevice(m_vkDevice, nullptr);
}

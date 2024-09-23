#pragma once
#include <bitset>
#include <vulkan/vulkan.h>
#include <vector>
#include "QueueFlags.h"
#include "Device.h"
#include <GLFW/glfw3.h>

extern const bool ENABLE_VALIDATION;

class Device;
class Instance
{
public:
	
	Instance()=delete;
	Instance(const char* applicationName, unsigned int additionalExtensionCount = 0, const char** additionalExtensions = nullptr);
	VkInstance GetVkInstance();
	VkPhysicalDevice GetPhysicalDevice();
	const QueueFamilyIndices& GetQueueFamilyIndices() const;
	const VkSurfaceCapabilitiesKHR& GetSurfaceCapabilities() const;

	const std::vector<VkSurfaceFormatKHR>& GetSurfaceFormats() const;
	const std::vector<VkPresentModeKHR>& GetPresentModes() const;

	uint32_t GetMemoryTypeIndex(uint32_t types, VkMemoryPropertyFlags properties) const;
	VkFormat GetSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

	void PickPhysicalDevice(std::vector<const char*> deviceExtensions, QueueFlagBits requiredQueues, VkSurfaceKHR surface = VK_NULL_HANDLE);
	Device* CreateDevice(QueueFlagBits requiredQueues);
		~Instance();

private:
	void InitDebugReport();
	VkInstance m_instance;

	VkDebugUtilsMessengerEXT m_DebugMessenger;
	std::vector<const char*> m_deviceExtensions;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	QueueFamilyIndices m_queueFamilyIndices;
	VkSurfaceCapabilitiesKHR m_surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
	std::vector<VkPresentModeKHR> m_presentMode;
	VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties;
};
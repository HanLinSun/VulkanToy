#include <set>
#include <stdexcept>
#include <vector>
#include "Headers/Vulkan/Instance.h"
#include "Tools.h"
#include <iostream>

#ifdef NDEBUG
const bool ENABLE_VALIDATION = false;
#else
const bool ENABLE_VALIDATION = true;
#endif


namespace 
{
    const std::vector<const char*> validationLayers = {
       "VK_LAYER_KHRONOS_validation"
    };

    std::vector<const char*> GetRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (ENABLE_VALIDATION) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;
    }


    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }
}

Instance::Instance(const char* applicationName, unsigned int additionalExtensionCount, const char** additionalExtensions)
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = applicationName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = GetRequiredExtensions();
    for (unsigned int i = 0; i < additionalExtensionCount; i++)
    {
        extensions.push_back(additionalExtensions[i]);
    }
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (ENABLE_VALIDATION) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    InitDebugReport();
}

void Instance::InitDebugReport() {
    if (ENABLE_VALIDATION) {
        // Specify details for callback
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;

        if ([&]() {
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
                if (func != nullptr) {
                    return func(m_instance, &createInfo, nullptr, &m_DebugMessenger);
                }
                else {
                    return VK_ERROR_EXTENSION_NOT_PRESENT;
                }
            }() != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug callback");
        }
    }
}

VkInstance Instance::GetVkInstance()
{
    return m_instance;
}

VkPhysicalDevice Instance::GetPhysicalDevice()
{
    return m_physicalDevice;
}

const VkSurfaceCapabilitiesKHR& Instance::GetSurfaceCapabilities() const
{
    return m_surfaceCapabilities;
}

const QueueFamilyIndices& Instance::GetQueueFamilyIndices() const
{
    return m_queueFamilyIndices;
}

const std::vector<VkSurfaceFormatKHR>& Instance::GetSurfaceFormats() const
{
    return m_surfaceFormats;
}

const std::vector<VkPresentModeKHR>& Instance::GetPresentModes() const
{
    return m_presentMode;
}

uint32_t Instance::GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const
{
    // Iterate over all memory types available for the device used in this example
    for (uint32_t i = 0; i < m_deviceMemoryProperties.memoryTypeCount; i++) {
        if ((typeBits & 1) == 1) {
            if ((m_deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        typeBits >>= 1;
    }
    throw std::runtime_error("Could not find a suitable memory type!");
}

VkFormat Instance::GetSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const {
    for (VkFormat format : candidates) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("Failed to find supported format");
}

namespace
{
    QueueFamilyIndices CheckDeviceQueueSupport(VkPhysicalDevice device, QueueFlagBits requiredQueues, VkSurfaceKHR surface = VK_NULL_HANDLE) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        VkQueueFlags requiredVulkanQueues = 0;
        if (requiredQueues[QueueFlags::Graphics])
        {
            requiredVulkanQueues |= VK_QUEUE_GRAPHICS_BIT;
        }
        if (requiredQueues[QueueFlags::Compute])
        {
            requiredVulkanQueues |= VK_QUEUE_COMPUTE_BIT;
        }
        if (requiredQueues[QueueFlags::Transfer])
        {
            requiredVulkanQueues |= VK_QUEUE_TRANSFER_BIT;
        }

        QueueFamilyIndices indices = {};
        indices.fill(-1);
        VkQueueFlags supportedQueues = 0;
        bool needsPresent = requiredQueues[QueueFlags::Present];
        bool presentSupported = false;

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueCount>0) {
                supportedQueues |= queueFamily.queueFlags;
            }
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices[QueueFlags::Graphics] = i;
            }
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                indices[QueueFlags::Compute] = i;
            }
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                indices[QueueFlags::Transfer] = i;
            }

            if (needsPresent)
            {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
                if (queueFamily.queueCount > 0 && presentSupport) {
                    presentSupported = true;
                    indices[QueueFlags::Present] = i;
                }
            }

            if ((requiredVulkanQueues & supportedQueues)==requiredVulkanQueues && (!needsPresent ||presentSupported)) {
                break;
            }
            i++;
        }
        return indices;
    }

    bool IsIndicesValid(QueueFamilyIndices indices)
    {
        return (indices[QueueFlags::Graphics] != -1) && (indices[QueueFlags::Present] != -1);
    }

    // Check the physical device for specified extension support
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> requiredExtensions) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensionSet(requiredExtensions.begin(), requiredExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensionSet.erase(extension.extensionName);
        }

        return requiredExtensionSet.empty();
    }

    VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice physicalDevice) {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }
}

void Instance::PickPhysicalDevice(std::vector<const char*> deviceExtensions, QueueFlagBits requiredQueues, VkSurfaceKHR surface)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    for (const auto& device : devices)
    {
        bool queueSupport = true;
        m_queueFamilyIndices = CheckDeviceQueueSupport(device, requiredQueues, surface);
        for (unsigned int i = 0; i < requiredQueues.size(); ++i) {
            if (requiredQueues[i]) {
                queueSupport &= (m_queueFamilyIndices[i] >= 0);
            }
        }

        if (requiredQueues[QueueFlags::Present])
        {
            // Get basic surface capabilities
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &m_surfaceCapabilities);

            // Query supported surface formats
            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

            if (formatCount != 0) {
                m_surfaceFormats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, m_surfaceFormats.data());
            }

            // Query supported presentation modes
            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                m_presentMode.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, m_presentMode.data());
            }
            // msaaSamples = getMaxUsableSampleCount();
            if (queueSupport &&
                CheckDeviceExtensionSupport(device, deviceExtensions) &&
                (!requiredQueues[QueueFlags::Present] || (!m_surfaceFormats.empty() && !m_presentMode.empty()))
                ) {
                m_physicalDevice = device;
                break;
            }
        }
    }
    this->m_deviceExtensions = deviceExtensions;
    if (m_physicalDevice == VK_NULL_HANDLE) 
    {
            throw std::runtime_error("Failed to find a suitable GPU");
    }
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_deviceMemoryProperties);
}


Device* Instance::CreateDevice(QueueFlagBits requiredQueues, VkPhysicalDeviceFeatures deviceFeatures)
{
    std::set<int> uniqueQueueFamilies;
    bool queueSupport = true;
    for (unsigned int i = 0; i < requiredQueues.size(); i++)
    {
        if (requiredQueues[i])
        {
            queueSupport &= (m_queueFamilyIndices[i] >= 0);
            uniqueQueueFamilies.insert(m_queueFamilyIndices[i]);
        }
    }

    if (!queueSupport)
    {
        throw std::runtime_error("Device does not support requested queues");
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    for (int queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // --- Create logical device ---
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    // Enable device-specific extensions and validation layers
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

    if (ENABLE_VALIDATION) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    VkDevice vkDevice;
    // Create logical device
    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &vkDevice) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }

    Device::Queues queues;
    for (unsigned int i = 0; i < requiredQueues.size(); ++i) {
        if (requiredQueues[i]) {
            vkGetDeviceQueue(vkDevice, m_queueFamilyIndices[i], 0, &queues[i]);
        }
    }
    Device* device = new Device(this, vkDevice, queues);
    device->m_commandPool = Tools::CreateCommandPool(device, QueueFlags::Graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    return device;
}

Instance::~Instance() {
    if (ENABLE_VALIDATION) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(m_instance, m_DebugMessenger, nullptr);
        }
    }
    vkDestroyInstance(m_instance, nullptr);
}

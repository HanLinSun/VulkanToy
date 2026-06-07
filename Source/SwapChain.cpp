#include <vector>
#include <Vulkan/SwapChain.h>
#include <Vulkan/Instance.h>
#include <Tools.h>

namespace {
    // Specify the color channel format and color space type
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        // VK_FORMAT_UNDEFINED indicates that the surface has no preferred format, so we can choose any
        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
            return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        }

        // Otherwise, choose a preferred combination
        for (const auto& availableFormat : availableFormats) {
            // Ideal format and color space
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        // Otherwise, return any format
        return availableFormats[0];
    }
    // Specify the presentation mode of the swap chain
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
            else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                bestMode = availablePresentMode;
            }
        }
        return bestMode;
    }
    // Specify the swap extent (resolution) of the swap chain
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
            return actualExtent;
        }
    }
}

SwapChain::SwapChain(std::shared_ptr<Device> device, VkSurfaceKHR vkSurface, unsigned int numBuffers,GLFWwindow* window)
    :m_device(device),m_vkSurface(vkSurface),m_numBuffers(numBuffers),m_window(window)
{
    Create();
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    m_imageAvailableSemaphores.resize(VK_MAX_FRAMES_LAG);
    m_renderFinishedSemaphores.resize(m_vkSwapChainImages.size());

    for (uint32_t i = 0; i < VK_MAX_FRAMES_LAG; i++)
    {
        vkCreateSemaphore(device->GetVkDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]);
    }
    for (uint32_t i = 0; i < m_vkSwapChainImages.size(); i++)
    {
        check_vk_result(vkCreateSemaphore(device->GetVkDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]));
    }
}
std::vector<VkImage> SwapChain::GetVkImages() const
{
    return m_vkSwapChainImages;
}

void SwapChain::Create()
{
    auto* instance = m_device->GetInstance();
    const auto& surfaceCapabilities = instance->GetSurfaceCapabilities();
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(instance->GetSurfaceFormats());
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(instance->GetPresentModes());
    VkExtent2D extent = ChooseSwapExtent(surfaceCapabilities, m_window);

    uint32_t imageCount = std::max(static_cast<uint32_t>(VK_MAX_FRAMES_LAG),surfaceCapabilities.minImageCount);

    if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    // --- Create swap chain ---
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    // Specify surface to be tied to
    createInfo.surface = m_vkSurface;

    // Add details of the swap chain
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const auto& queueFamilyIndices = instance->GetQueueFamilyIndices();
    if (queueFamilyIndices[QueueFlags::Graphics] != queueFamilyIndices[QueueFlags::Present]) {
        // Images can be used across multiple queue families without explicit ownership transfers
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        unsigned int indices[] = {
            static_cast<unsigned int>(queueFamilyIndices[QueueFlags::Graphics]),
            static_cast<unsigned int>(queueFamilyIndices[QueueFlags::Present])
        };
        createInfo.pQueueFamilyIndices = indices;
    }
    else {
        // An image is owned by one queue family at a time and ownership must be explicitly transfered between uses
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    check_vk_result(vkCreateSwapchainKHR(m_device->GetVkDevice(), &createInfo, nullptr, &m_vkSwapChain));

    // --- Retrieve swap chain images ---
    vkGetSwapchainImagesKHR(m_device->GetVkDevice(), m_vkSwapChain, &imageCount, nullptr);
    m_vkSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device->GetVkDevice(), m_vkSwapChain, &imageCount, m_vkSwapChainImages.data());

    m_vkSwapChainImageFormat = surfaceFormat.format;
    m_vkSwapChainExtent = extent;
}

void SwapChain::Destroy() {
    vkDestroySwapchainKHR(m_device->GetVkDevice(), m_vkSwapChain, nullptr);
}

VkSwapchainKHR SwapChain::GetVkSwapChain() const {
    return m_vkSwapChain;
}

VkFormat SwapChain::GetVkImageFormat() const {
    return m_vkSwapChainImageFormat;
}

VkExtent2D SwapChain::GetVkExtent() const {
    return m_vkSwapChainExtent;
}

uint32_t SwapChain::GetCurrentImageIndex() const {
    return m_imageIndex;
}

uint32_t SwapChain::GetCount() const {
    return static_cast<uint32_t>(m_vkSwapChainImages.size());
}

VkImage SwapChain::GetVkImage(uint32_t index) const {
    return m_vkSwapChainImages[index];
}

VkSemaphore SwapChain::GetImageAvailableVkSemaphore(int currentImage) const {
    return m_imageAvailableSemaphores[currentImage];
}

VkResult SwapChain::QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
{
    VkSwapchainKHR swapChains[] = { GetVkSwapChain() };

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    // Check if a wait semaphore has been specified to wait for before presenting the image
    if (waitSemaphore != VK_NULL_HANDLE)
    {
        presentInfo.pWaitSemaphores = &waitSemaphore;
        presentInfo.waitSemaphoreCount = 1;
    }
    return vkQueuePresentKHR(queue, &presentInfo);
}

VkSemaphore SwapChain::GetRenderFinishedVkSemaphore(int imageIndex) const {
    return m_renderFinishedSemaphores[imageIndex];
}

void SwapChain::Recreate() {
    Destroy();
    Create();
}

bool SwapChain::AcquireNextImage(int currentFrame) {

    if (ENABLE_VALIDATION) {
        // the validation layer implementation expects the application to explicitly synchronize with the GPU
        vkQueueWaitIdle(m_device->GetQueue(QueueFlags::Present));
    }
    VkResult result = vkAcquireNextImageKHR(m_device->GetVkDevice(), m_vkSwapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &m_imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image");
    }

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        Recreate();
        return false;
    }

    return true;
}

void SwapChain::DestroyVKResources()
{
    for (int i = 0; i < m_imageAvailableSemaphores.size(); i++)
    {
        vkDestroySemaphore(m_device->GetVkDevice(), m_imageAvailableSemaphores[i], nullptr);
    }
    for (int i = 0; i < m_renderFinishedSemaphores.size(); i++)
    {
        vkDestroySemaphore(m_device->GetVkDevice(), m_renderFinishedSemaphores[i], nullptr);
    }
   
    Destroy();
}

SwapChain::~SwapChain() {

}

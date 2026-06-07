#pragma once
#include <memory>
#include <vector>
#include <Vulkan/Device.h>
#include <glfw/glfw3.h>

#define VK_MAX_FRAMES_LAG (3)

class Device;
class SwapChain {
    friend class Device;

public:
    VkSwapchainKHR GetVkSwapChain() const;
    VkFormat GetVkImageFormat() const;
    VkExtent2D GetVkExtent() const;
    uint32_t GetCurrentImageIndex() const;
    uint32_t GetCount() const;
    VkImage GetVkImage(uint32_t index) const;
    std::vector<VkImage> GetVkImages() const;

    void DestroyVKResources();
    VkSemaphore GetImageAvailableVkSemaphore(int currentImage) const;
    VkSemaphore GetRenderFinishedVkSemaphore(int imageIndex) const;
    VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);
    void Recreate();
    bool AcquireNextImage(int currentFrame);

    SwapChain(std::shared_ptr<Device> device, VkSurfaceKHR vkSurface, unsigned int numBuffers, GLFWwindow* window);
    ~SwapChain();

private:

    void Create();
    void Destroy();

    std::shared_ptr<Device> m_device;
    VkSurfaceKHR m_vkSurface;
    GLFWwindow* m_window;

    unsigned int m_numBuffers;

    VkSwapchainKHR m_vkSwapChain;
    std::vector<VkImage> m_vkSwapChainImages;
    VkFormat m_vkSwapChainImageFormat;
    VkExtent2D m_vkSwapChainExtent;
    uint32_t m_imageIndex = 0; //swapChain available image index

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
};

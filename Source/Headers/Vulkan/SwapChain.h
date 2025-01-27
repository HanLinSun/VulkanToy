#pragma once
#include <memory>
#include <vector>
#include <Vulkan/Device.h>
#include <glfw/glfw3.h>

class Device;
class SwapChain {
    friend class Device;

public:
    VkSwapchainKHR GetVkSwapChain() const;
    VkFormat GetVkImageFormat() const;
    VkExtent2D GetVkExtent() const;
    uint32_t GetIndex() const;
    uint32_t GetCount() const;
    VkImage GetVkImage(uint32_t index) const;
    std::vector<VkImage> GetVkImages() const;

    void DestroyVKResources();
    VkSemaphore GetImageAvailableVkSemaphore() const;
    VkSemaphore GetRenderFinishedVkSemaphore() const;
    VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);
    void Recreate();
    bool Acquire();
    bool Present();

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
    uint32_t m_imageIndex = 0;

    VkSemaphore m_imageAvailableSemaphore;
    VkSemaphore m_renderFinishedSemaphore;
};

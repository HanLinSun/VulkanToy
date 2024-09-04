#pragma once

#include <vector>
#include "Device.h"

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

    VkSemaphore GetImageAvailableVkSemaphore() const;
    VkSemaphore GetRenderFinishedVkSemaphore() const;

    void Recreate();
    bool Acquire();
    bool Present();

    ~SwapChain();

private:
    SwapChain(Device* device, VkSurfaceKHR vkSurface, unsigned int numBuffers);
    void Create();
    void Destroy();

    Device* m_device;
    VkSurfaceKHR m_vkSurface;

    unsigned int m_numBuffers;

    VkSwapchainKHR m_vkSwapChain;
    std::vector<VkImage> m_vkSwapChainImages;
    VkFormat m_vkSwapChainImageFormat;
    VkExtent2D m_vkSwapChainExtent;
    uint32_t m_imageIndex = 0;

    VkSemaphore m_imageAvailableSemaphore;
    VkSemaphore m_renderFinishedSemaphore;
};

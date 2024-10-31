/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*   Overvoorde, A. (2017). Khronos Vulkan Tutorial. [online] Vulkan.org. Available at: https://docs.vulkan.org/tutorial/latest/00_Introduction.html (Accessed 07 June 2024).
*/
#pragma once

#include "VRE_Device.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <memory>

namespace VRE {

    class VRE_SwapChain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    
        VRE_SwapChain(VRE_Device& deviceRef, VkExtent2D extent);
        VRE_SwapChain(VRE_Device& deviceRef, VkExtent2D extent, std::shared_ptr<VRE_SwapChain> previous);
        ~VRE_SwapChain();
    
        VRE_SwapChain(const VRE_SwapChain &) = delete;
        VRE_SwapChain& operator=(const VRE_SwapChain &) = delete;
    
        void Init();
        VkFramebuffer GetFrameBuffer(int index) { return mSwapChainFramebuffers[index]; }
        VkRenderPass GetRenderPass() { return mRenderPass; }
        VkImageView GetImageView(int index) { return mSwapChainImageViews[index]; }
        size_t GetImageCount() { return mSwapChainImages.size(); }
        VkFormat GetSwapChainImageFormat() { return mSwapChainImageFormat; }
        VkExtent2D GetSwapChainExtent() { return mSwapChainExtent; }
        uint32_t GetWidth() { return mSwapChainExtent.width; }
        uint32_t GetHeight() { return mSwapChainExtent.height; }
    
        float GetExtentAspectRatio() { return static_cast<float>(mSwapChainExtent.width) / static_cast<float>(mSwapChainExtent.height); }

        VkFormat FindDepthFormat();
    
        VkResult AcquireNextImage(uint32_t *imageIndex);
        VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

        bool CompareSwapFormats(const VRE_SwapChain& swapChain) const {
            return swapChain.mSwapChainDepthFormat == mSwapChainDepthFormat && swapChain.mSwapChainImageFormat == mSwapChainImageFormat;
        }
    
    private:
        void CreateSwapChain();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();
    
        // Helper functions
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    
        VkFormat mSwapChainImageFormat;
        VkFormat mSwapChainDepthFormat;
        VkExtent2D mSwapChainExtent;
    
        std::vector<VkFramebuffer> mSwapChainFramebuffers;
        VkRenderPass mRenderPass;
    
        std::vector<VkImage> mDepthImages;
        std::vector<VkDeviceMemory> mDepthImageMemorys;
        std::vector<VkImageView> mDepthImageViews;
        std::vector<VkImage> mSwapChainImages;
        std::vector<VkImageView> mSwapChainImageViews;
    
        VRE_Device &mDevice;
        VkExtent2D mWindowExtent;
    
        VkSwapchainKHR mSwapChain;
        std::shared_ptr<VRE_SwapChain> mOldSwapChain;
    
        std::vector<VkSemaphore> mImageAvailableSemaphores;
        std::vector<VkSemaphore> mRenderFinishedSemaphores;
        std::vector<VkFence> mInFlightFences;
        std::vector<VkFence> mImagesInFlight;
        size_t mCurrentFrame = 0;
    };
}
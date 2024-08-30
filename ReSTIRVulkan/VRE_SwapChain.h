#pragma once

#include "VRE_Device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
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
        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
        VkRenderPass getRenderPass() { return renderPass; }
        VkImageView getImageView(int index) { return swapChainImageViews[index]; }
        size_t imageCount() { return swapChainImages.size(); }
        VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
        VkExtent2D getSwapChainExtent() { return swapChainExtent; }
        uint32_t width() { return swapChainExtent.width; }
        uint32_t height() { return swapChainExtent.height; }
    
        float extentAspectRatio() {
          return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }
        VkFormat findDepthFormat();
    
        VkResult acquireNextImage(uint32_t *imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

        bool CompareSwapFormats(const VRE_SwapChain& swapChain) const {
            return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
                   swapChain.swapChainImageFormat == swapChainImageFormat;
        }
    
    private:
        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();
    
        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    
        VkFormat swapChainImageFormat;
        VkFormat swapChainDepthFormat;
        VkExtent2D swapChainExtent;
    
        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;
    
        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
    
        VRE_Device &device;
        VkExtent2D windowExtent;
    
        VkSwapchainKHR swapChain;
        std::shared_ptr<VRE_SwapChain> oldSwapChain;
    
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
    };
}
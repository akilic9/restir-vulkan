/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*   Overvoorde, A. (2017). Khronos Vulkan Tutorial. [online] Vulkan.org. Available at: https://docs.vulkan.org/tutorial/latest/00_Introduction.html (Accessed 07 June 2024).
*/
#pragma once

#include "VRE_Window.h"
#include <string>
#include <vector>

namespace VRE {

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR mCapabilities;
        std::vector<VkSurfaceFormatKHR> mFormats;
        std::vector<VkPresentModeKHR> mPresentModes;
    };
    
    struct QueueFamilyIndices {
        uint32_t mGraphicsFamily;
        uint32_t mPresentFamily;
        bool mGraphicsFamilyHasValue = false;
        bool mPresentFamilyHasValue = false;

        bool IsComplete() { return mGraphicsFamilyHasValue && mPresentFamilyHasValue; }
    };
    
    class VRE_Device
    {
    public:

#ifdef NDEBUG
        const bool mEnableValidationLayers = false;
#else
        const bool mEnableValidationLayers = true;
#endif

        VRE_Device(VRE_Window &window);
        ~VRE_Device();
        
        VRE_Device(const VRE_Device &) = delete;
        VRE_Device& operator=(const VRE_Device &) = delete;
        VRE_Device(VRE_Device &&) = delete;
        VRE_Device &operator=(VRE_Device &&) = delete;
        
        VkCommandPool GetCommandPool() { return mCommandPool; }
        VkDevice GetVkDevice() { return mVkDevice; }
        VkPhysicalDevice GetPhysicalDevice() { return mPhysicalDevice; }
        VkSurfaceKHR Surface() { return mVkSurface; }
        VkQueue GraphicsQueue() { return mGraphicsQueue; }
        VkQueue PresentQueue() { return mPresentQueue; }
        
        SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(mPhysicalDevice); }
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(mPhysicalDevice); }
        VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
    
        void CreateImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
    
        VkPhysicalDeviceProperties mProperties;
        
    private:
        void CreateInstance();
        void SetupDebugMessenger();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateCommandPool();
    
        bool IsDeviceSuitable(VkPhysicalDevice device);
        std::vector<const char *> GetRequiredExtensions();
        bool CheckValidationLayerSupport();
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        void HasGflwRequiredInstanceExtensions();
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    
        VkInstance mInstance;
        VkDebugUtilsMessengerEXT mDebugMessenger;
        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
        VRE_Window &mWindow;
        VkCommandPool mCommandPool;
    
        VkDevice mVkDevice;
        VkSurfaceKHR mVkSurface;
        VkQueue mGraphicsQueue;
        VkQueue mPresentQueue;
    
        const std::vector<const char *> mValidationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> mDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };
}
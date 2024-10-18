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
        
        // Not copyable or movable
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
        
        // Buffer Helper Functions
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
    
        // Helper functions
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
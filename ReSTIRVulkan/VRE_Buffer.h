/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*   Overvoorde, A. (2017). Khronos Vulkan Tutorial. [online] Vulkan.org. Available at: https://docs.vulkan.org/tutorial/latest/00_Introduction.html (Accessed 07 June 2024).
*/
#pragma once
#include "VRE_Device.h"

namespace VRE {
    class VRE_Buffer
    {
    public:
        VRE_Buffer(VRE_Device& device, VkDeviceSize instanceSize, uint32_t instanceCount,
                  VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
                  VkDeviceSize minOffsetAlignment = 1);
        ~VRE_Buffer();

        VRE_Buffer(const VRE_Buffer&) = delete;
        VRE_Buffer& operator=(const VRE_Buffer&) = delete;

        VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Unmap();

        void WriteToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void WriteToIndex(void* data, int index);
        VkResult FlushIndex(int index);
        VkDescriptorBufferInfo DescriptorInfoForIndex(int index);
        VkResult InvalidateIndex(int index);

        VkBuffer GetBuffer() const { return mBuffer; }
        void* GetMappedMemory() const { return mMapped; }
        uint32_t GetInstanceCount() const { return mInstanceCount; }
        VkDeviceSize GetInstanceSize() const { return mInstanceSize; }
        VkDeviceSize GetAlignmentSize() const { return mAlignmentSize; }
        VkBufferUsageFlags GetUsageFlags() const { return mUsageFlags; }
        VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return mMemoryPropertyFlags; }
        VkDeviceSize GetBufferSize() const { return mBufferSize; }

    private:
        static VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

        VRE_Device& mDevice;
        void* mMapped = nullptr;
        VkBuffer mBuffer = VK_NULL_HANDLE;
        VkDeviceMemory mMemory = VK_NULL_HANDLE;

        VkDeviceSize mBufferSize;
        uint32_t mInstanceCount;
        VkDeviceSize mInstanceSize;
        VkDeviceSize mAlignmentSize;
        VkBufferUsageFlags mUsageFlags;
        VkMemoryPropertyFlags mMemoryPropertyFlags;
    };
}
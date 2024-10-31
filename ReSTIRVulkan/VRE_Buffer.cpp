/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*   Overvoorde, A. (2017). Khronos Vulkan Tutorial. [online] Vulkan.org. Available at: https://docs.vulkan.org/tutorial/latest/00_Introduction.html (Accessed 07 June 2024).
*/

/*
 * Encapsulates a Vulkan buffer.
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include "VRE_Buffer.h"
#include <cassert>
#include <cstring>

VRE::VRE_Buffer::VRE_Buffer(VRE_Device& device, VkDeviceSize instanceSize,
                            uint32_t instanceCount, VkBufferUsageFlags usageFlags,
                            VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment)
    : mDevice(device)
    , mInstanceSize(instanceSize)
    , mInstanceCount(instanceCount)
    , mUsageFlags(usageFlags)
    , mMemoryPropertyFlags(memoryPropertyFlags)
{
    mAlignmentSize = GetAlignment(instanceSize, minOffsetAlignment);
    mBufferSize = mAlignmentSize * mInstanceCount;
    mDevice.CreateBuffer(mBufferSize, usageFlags, memoryPropertyFlags, mBuffer, mMemory);
}

VRE::VRE_Buffer::~VRE_Buffer()
{
    Unmap();
    vkDestroyBuffer(mDevice.GetVkDevice(), mBuffer, nullptr);
    vkFreeMemory(mDevice.GetVkDevice(), mMemory, nullptr);
}

VkResult VRE::VRE_Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
    assert(mBuffer && mMemory && "Map function calles on buffer before create function.");
    return vkMapMemory(mDevice.GetVkDevice(), mMemory, offset, size, 0, &mMapped);
}

void VRE::VRE_Buffer::Unmap()
{
    if (mMapped) {
        vkUnmapMemory(mDevice.GetVkDevice(), mMemory);
        mMapped = nullptr;
    }
}

void VRE::VRE_Buffer::WriteToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset)
{
    assert(mMapped && "Cannot copy to unmapped buffer.");

    if (size == VK_WHOLE_SIZE)
        memcpy(mMapped, data, mBufferSize);
    else {
        char* memOffset = (char*)mMapped;
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

VkResult VRE::VRE_Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = mMemory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(mDevice.GetVkDevice(), 1, &mappedRange);
}

VkDescriptorBufferInfo VRE::VRE_Buffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset)
{
    return VkDescriptorBufferInfo{mBuffer, offset, size};
}

VkResult VRE::VRE_Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange;
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = mMemory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(mDevice.GetVkDevice(), 1, &mappedRange);
}

void VRE::VRE_Buffer::WriteToIndex(void* data, int index)
{
    WriteToBuffer(data, mInstanceSize, index * mAlignmentSize);
}

VkResult VRE::VRE_Buffer::FlushIndex(int index)
{
    return Flush(mAlignmentSize, index * mAlignmentSize);
}

VkDescriptorBufferInfo VRE::VRE_Buffer::DescriptorInfoForIndex(int index)
{
    return DescriptorInfo(mAlignmentSize, index * mAlignmentSize);
}

VkResult VRE::VRE_Buffer::InvalidateIndex(int index)
{
    return Invalidate(mAlignmentSize, index * mAlignmentSize);
}

VkDeviceSize VRE::VRE_Buffer::GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
{
    if (minOffsetAlignment > 0)
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);

    return instanceSize;
}
/*
 * Encapsulates a vulkan buffer
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
    mBufferSize = mAlignmentSize * instanceCount;
    mDevice.CreateBuffer(mBufferSize, usageFlags, memoryPropertyFlags, mBuffer, mMemory);
}

VRE::VRE_Buffer::~VRE_Buffer()
{
    Unmap();
    vkDestroyBuffer(mDevice.GetVkDevice(), mBuffer, nullptr);
    vkFreeMemory(mDevice.GetVkDevice(), mMemory, nullptr);
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
 * buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer mapping call
 */
VkResult VRE::VRE_Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
    assert(mBuffer && mMemory && "Map function calles on buffer before create function.");
    return vkMapMemory(mDevice.GetVkDevice(), mMemory, offset, size, 0, &mMapped);
}

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void VRE::VRE_Buffer::Unmap()
{
    if (mMapped) {
        vkUnmapMemory(mDevice.GetVkDevice(), mMemory);
        mMapped = nullptr;
    }
}

/**
 * Copies the specified data to the mapped buffer. Default value writes whole buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
 * range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
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

/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
 * complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
VkResult VRE::VRE_Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = mMemory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(mDevice.GetVkDevice(), 1, &mappedRange);
}

/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
VkDescriptorBufferInfo VRE::VRE_Buffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset)
{
    return VkDescriptorBufferInfo{mBuffer, offset, size};
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
VkResult VRE::VRE_Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange;
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = mMemory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(mDevice.GetVkDevice(), 1, &mappedRange);
}

/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void VRE::VRE_Buffer::WriteToIndex(void* data, int index)
{
    WriteToBuffer(data, mInstanceSize, index * mAlignmentSize);
}

/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
 *
 * @param index Used in offset calculation
 *
 */
VkResult VRE::VRE_Buffer::FlushIndex(int index)
{
    return Flush(mAlignmentSize, index * mAlignmentSize);
}

/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignmentSize
 *
 * @return VkDescriptorBufferInfo for instance at index
 */
VkDescriptorBufferInfo VRE::VRE_Buffer::DescriptorInfoForIndex(int index)
{
    return DescriptorInfo(mAlignmentSize, index * mAlignmentSize);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param index Specifies the region to invalidate: index * alignmentSize
 *
 * @return VkResult of the invalidate call
 */
VkResult VRE::VRE_Buffer::InvalidateIndex(int index)
{
    return Invalidate(mAlignmentSize, index * mAlignmentSize);
}

/**
 * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
 *
 * @param instanceSize The size of an instance
 * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
 * minUniformBufferOffsetAlignment)
 *
 * @return VkResult of the buffer mapping call
 */
VkDeviceSize VRE::VRE_Buffer::GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
{
    if (minOffsetAlignment > 0) {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
}
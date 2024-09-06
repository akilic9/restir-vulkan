#include "VRE_Model.h"
#include <cassert>
#include <cstring>

VRE::VRE_Model::VRE_Model(VRE_Device& device, const ModelData& data)
    : mDevice(device)
    , mHasIndexBuffer(false)
{
    CreateVertexBuffers(data.mVertices);
    CreateIndexBuffer(data.mIndices);
}

VRE::VRE_Model::~VRE_Model()
{
    vkDestroyBuffer(mDevice.device(), mVertexBuffer, nullptr);
    vkFreeMemory(mDevice.device(), mVertexBufferMemory, nullptr);

    if (mHasIndexBuffer) {
        vkDestroyBuffer(mDevice.device(), mIndexBuffer, nullptr);
        vkFreeMemory(mDevice.device(), mIndexBufferMemory, nullptr);
    }
}

void VRE::VRE_Model::Bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = { mVertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    if (mHasIndexBuffer)
        vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void VRE::VRE_Model::Draw(VkCommandBuffer commandBuffer)
{
    if (mHasIndexBuffer)
        vkCmdDrawIndexed(commandBuffer, mIndexCount, 1, 0, 0, 0);
    else
        vkCmdDraw(commandBuffer, mVertexCount, 1, 0, 0);
}

void VRE::VRE_Model::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
    mVertexCount = static_cast<uint32_t>(vertices.size());

    assert(mVertexCount >= 3 && "Vertex count must be at least 3!");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * mVertexCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    mDevice.createBuffer(bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer,
                         stagingBufferMemory);
    
    void* data;
    vkMapMemory(mDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(mDevice.device(), stagingBufferMemory);

    mDevice.createBuffer(bufferSize,
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         mVertexBuffer,
                         mVertexBufferMemory);

    mDevice.copyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

    vkDestroyBuffer(mDevice.device(), stagingBuffer, nullptr);
    vkFreeMemory(mDevice.device(), stagingBufferMemory, nullptr);
}

void VRE::VRE_Model::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
    mIndexCount = static_cast<uint32_t>(indices.size());
    mHasIndexBuffer = mIndexCount > 0;

    if (!mHasIndexBuffer)
        return;

    VkDeviceSize bufferSize = sizeof(indices[0]) * mIndexCount;
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    mDevice.createBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void* data;
    vkMapMemory(mDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(mDevice.device(), stagingBufferMemory);

    mDevice.createBuffer(bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mIndexBuffer,
        mIndexBufferMemory);

    mDevice.copyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

    vkDestroyBuffer(mDevice.device(), stagingBuffer, nullptr);
    vkFreeMemory(mDevice.device(), stagingBufferMemory, nullptr);
}

std::vector<VkVertexInputBindingDescription> VRE::VRE_Model::Vertex::GetBindingDesc()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> VRE::VRE_Model::Vertex::GetAttributeDesc()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions; 
}

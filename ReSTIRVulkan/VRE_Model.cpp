#include "VRE_Model.h"
#include <cassert>
#include <cstring>

VRE::VRE_Model::VRE_Model(VRE_Device& device, const std::vector<Vertex>& vertices)
    : mDevice(device)
{
    CreateVertexBuffers(vertices);
}

VRE::VRE_Model::~VRE_Model()
{
    vkDestroyBuffer(mDevice.device(), mVertexBuffer, nullptr);
    vkFreeMemory(mDevice.device(), mVertexBufferMemory, nullptr);
}

void VRE::VRE_Model::Bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = { mVertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void VRE::VRE_Model::Draw(VkCommandBuffer commandBuffer)
{
    vkCmdDraw(commandBuffer, mVertexCount, 1, 0, 0);
}

void VRE::VRE_Model::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
    mVertexCount = static_cast<uint32_t>(vertices.size());

    assert(mVertexCount >= 3 && "Vertex count must be at least 3!");

    VkDeviceSize bufferSize = sizeof(vertices[0]) * mVertexCount;
    mDevice.createBuffer(bufferSize,
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         mVertexBuffer,
                         mVertexBufferMemory);
    void* data;
    vkMapMemory(mDevice.device(), mVertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(mDevice.device(), mVertexBufferMemory);
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
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions; 
}

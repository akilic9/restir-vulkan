#include "VRE_Model.h"
#include "VRE_Utilities.h"
#include <cassert>
#include <cstring>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>

namespace std {
    template <>
    struct hash<VRE::VRE_Model::Vertex> {
        size_t operator()(VRE::VRE_Model::Vertex const& vertex) const {
            size_t seed = 0;
            VRE::hashCombine(seed, vertex.mPosition, vertex.mColor, vertex.mNormal, vertex.mUV);
            return seed;
        }
    };
}  // namespace std

VRE::VRE_Model::VRE_Model(VRE_Device& device, const ModelData& data)
    : mDevice(device)
    , mHasIndexBuffer(false)
{
    CreateVertexBuffers(data.mVertices);
    CreateIndexBuffer(data.mIndices);
}

VRE::VRE_Model::~VRE_Model() {}

std::unique_ptr<VRE::VRE_Model> VRE::VRE_Model::CreateModel(VRE_Device& device, const std::string& filePath)
{
    ModelData data{};

    data.LoadModel(filePath);

    return std::make_unique<VRE_Model>(device, data);
}

void VRE::VRE_Model::Bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = { mVertexBuffer->GetBuffer()};
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    if (mHasIndexBuffer)
        vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
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

    uint32_t vertexSize = sizeof(vertices[0]);

    VRE_Buffer stagingBuffer(mDevice, vertexSize, mVertexCount,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.Map();
    stagingBuffer.WriteToBuffer((void*)vertices.data());

    mVertexBuffer = std::make_unique<VRE_Buffer>(mDevice, vertexSize, mVertexCount,
                                                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    mDevice.copyBuffer(stagingBuffer.GetBuffer(), mVertexBuffer->GetBuffer(), bufferSize);
}

void VRE::VRE_Model::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
    mIndexCount = static_cast<uint32_t>(indices.size());
    mHasIndexBuffer = mIndexCount > 0;

    if (!mHasIndexBuffer)
        return;

    VkDeviceSize bufferSize = sizeof(indices[0]) * mIndexCount;
    uint32_t indexSize = sizeof(indices[0]);

    VRE_Buffer stagingBuffer(mDevice, indexSize, mIndexCount,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.Map();
    stagingBuffer.WriteToBuffer((void*)indices.data());

    mIndexBuffer = std::make_unique<VRE_Buffer>(mDevice, indexSize, mIndexCount,
                                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    mDevice.copyBuffer(stagingBuffer.GetBuffer(), mIndexBuffer->GetBuffer(), bufferSize);
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
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, mPosition)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, mColor)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, mNormal)});
    attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, mUV)});

    return attributeDescriptions; 
}

void VRE::VRE_Model::ModelData::LoadModel(const std::string& filePath)
{
    tinyobj::attrib_t attribute;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning, error;

    if (!tinyobj::LoadObj(&attribute, &shapes, &materials, &warning, &error, filePath.c_str())) {
        throw std::runtime_error(warning + error);
    }

    mVertices.clear();
    mIndices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            if (index.vertex_index >= 0) {
                vertex.mPosition = {
                    attribute.vertices[3 * index.vertex_index + 0],
                    attribute.vertices[3 * index.vertex_index + 1],
                    attribute.vertices[3 * index.vertex_index + 2],
                };

                vertex.mColor = {
                    attribute.colors[3 * index.vertex_index + 0],
                    attribute.colors[3 * index.vertex_index + 1],
                    attribute.colors[3 * index.vertex_index + 2],
                };
            }

            if (index.normal_index >= 0) {
                vertex.mNormal = {
                    attribute.normals[3 * index.normal_index + 0],
                    attribute.normals[3 * index.normal_index + 1],
                    attribute.normals[3 * index.normal_index + 2],
                };
            }

            if (index.texcoord_index >= 0) {
                vertex.mUV = {
                    attribute.texcoords[2 * index.texcoord_index + 0],
                    attribute.texcoords[2 * index.texcoord_index + 1],
                };
            }

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(mVertices.size());
                mVertices.push_back(vertex);
            }
            mIndices.push_back(uniqueVertices[vertex]);
        }
    }
}

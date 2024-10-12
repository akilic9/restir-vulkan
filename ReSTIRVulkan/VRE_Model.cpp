#include "VRE_Model.h"
#include <cassert>
#include <cstring>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

VRE::VRE_Model::VRE_Model(VRE_Device& device, const std::string& filePath)
    : mDevice(device)
    , mVertexBuffer(nullptr)
    , mVertexCount(0)
    , mHasIndexBuffer(false)
    , mIndexBuffer(nullptr)
    , mIndexCount(0)
{
    VRE::ModelData modelData{};
    LoadModel(filePath, modelData);
    CreateVertexBuffers(modelData.mVertices);
    CreateIndexBuffer(modelData.mIndices);
}

VRE::VRE_Model::~VRE_Model() {}

std::unique_ptr<VRE::VRE_Model> VRE::VRE_Model::CreateModel(VRE_Device& device, const std::string& filePath)
{
    return std::make_unique<VRE_Model>(device, filePath);
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

    VRE_Buffer stagingBuffer(mDevice, vertexSize, mVertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.Map();
    stagingBuffer.WriteToBuffer((void*)vertices.data());

    mVertexBuffer = std::make_unique<VRE_Buffer>(mDevice, vertexSize, mVertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    mDevice.CopyBuffer(stagingBuffer.GetBuffer(), mVertexBuffer->GetBuffer(), bufferSize);
}

void VRE::VRE_Model::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
    mIndexCount = static_cast<uint32_t>(indices.size());
    mHasIndexBuffer = mIndexCount > 0;

    if (!mHasIndexBuffer)
        return;

    VkDeviceSize bufferSize = sizeof(indices[0]) * mIndexCount;
    uint32_t indexSize = sizeof(indices[0]);

    VRE_Buffer stagingBuffer(mDevice, indexSize, mIndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.Map();
    stagingBuffer.WriteToBuffer((void*)indices.data());

    mIndexBuffer = std::make_unique<VRE_Buffer>(mDevice, indexSize, mIndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    mDevice.CopyBuffer(stagingBuffer.GetBuffer(), mIndexBuffer->GetBuffer(), bufferSize);
}

void VRE::VRE_Model::LoadModel(const std::string& filePath, ModelData& data)
{
    tinyobj::attrib_t attribute;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning, error;

    if (!tinyobj::LoadObj(&attribute, &shapes, &materials, &warning, &error, filePath.c_str()))
        throw std::runtime_error(warning + error);

    data.mVertices.clear();
    data.mIndices.clear();

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
                vertex.mTexCoord = {
                    attribute.texcoords[2 * index.texcoord_index + 0],
                    attribute.texcoords[2 * index.texcoord_index + 1],
                };
            }

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(data.mVertices.size());
                data.mVertices.push_back(vertex);
            }
            data.mIndices.push_back(uniqueVertices[vertex]);
        }
    }
}

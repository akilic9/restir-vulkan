#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT

#include "VRE_glTFModel.h"
#include <iostream>
#include <gtc/type_ptr.hpp>

VRE::VRE_glTFModel::VRE_glTFModel(VRE_Device& device, const std::string& fileFolder, const std::string& fileName)
    : mDevice(device)
    , mFileFolder(fileFolder)
    , mFileName(fileName)
    , mIndexCount(0)
    , mVertexCount(0)
{}

VRE::VRE_glTFModel::~VRE_glTFModel()
{}

void VRE::VRE_glTFModel::LoadModel()
{
    tinygltf::Model model;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;

    std::string filePath = mFileFolder;

    if (!gltfContext.LoadASCIIFromFile(&model, &error, &warning, filePath.append(mFileName).append(".gltf")))
        throw std::runtime_error("Failed to load glTF file!");

    LoadTextures(model);
    LoadMaterials(model);

    const tinygltf::Scene& scene = model.scenes[0];

    ModelData data{};

    for (size_t i = 0; i < scene.nodes.size(); i++) {
        const tinygltf::Node node = model.nodes[scene.nodes[i]];
        LoadNode(nullptr, node, model, data);
    }

    CreateVertexBuffers(data.mVertices);
    CreateIndexBuffer(data.mIndices);
}

void VRE::VRE_glTFModel::LoadTextures(tinygltf::Model& model)
{
    for (auto& image : model.images) {
        std::string filePath = mFileFolder;
        filePath.append(image.name).append(image.uri);
        mTextures.push_back(std::move(VRE_Texture::CreateTexture(mDevice, filePath)));
    }
}

void VRE::VRE_glTFModel::LoadMaterials(tinygltf::Model& model)
{
    mMaterials.resize(model.materials.size());
    for (size_t i = 0; i < model.materials.size(); i++) {

        tinygltf::Material inputMat = model.materials[i];

        if (inputMat.values.find("baseColorFactor") != inputMat.values.end()) {
            mMaterials[i].mBaseColorFactor = glm::make_vec4(inputMat.values["baseColorFactor"].ColorFactor().data());
        }

        if (inputMat.values.find("baseColorTexture") != inputMat.values.end()) {
            mMaterials[i].mBaseColorTextureIndex = inputMat.values["baseColorTexture"].TextureIndex();
        }
    }

    mTextureIndices.resize(model.textures.size());
    for (size_t i = 0; i < model.textures.size(); i++) {
        mTextureIndices[i] = model.textures[i].source;
    }
}

void VRE::VRE_glTFModel::LoadNode(glTFNode* parent, const tinygltf::Node& inputNode, const tinygltf::Model& model, ModelData& data)
{
    glTFNode* node = new glTFNode{};
    node->mParent = parent;

    // Node with children
    if (inputNode.children.size() > 0)
        for (size_t i = 0; i < inputNode.children.size(); i++)
            LoadNode(node, model.nodes[inputNode.children[i]], model, data);

    // If the node contains mesh data, we load vertices and indices from the buffers
    // In glTF this is done via accessors and buffer views
    if (inputNode.mesh > -1) {
        const tinygltf::Mesh mesh = model.meshes[inputNode.mesh];

        // Iterate through all primitives of this node's mesh
        for (size_t i = 0; i < mesh.primitives.size(); i++) {
            const tinygltf::Primitive& inputPrimitive = mesh.primitives[i];
            uint32_t firstIndex = static_cast<uint32_t>(mIndexCount);
            uint32_t vertexStart = static_cast<uint32_t>(mVertexCount);
            uint32_t indexCount = 0;

            // Vertices
            {
                const float* positionBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;
                const float* coloursBuffer = nullptr;
                size_t vertexCount = 0;

                // Get buffer data for vertex positions
                if (inputPrimitive.attributes.find("POSITION") != inputPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[inputPrimitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    positionBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    vertexCount = accessor.count;
                }

                // Get buffer data for vertex normals
                if (inputPrimitive.attributes.find("NORMAL") != inputPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[inputPrimitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    normalsBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                // Get buffer data for vertex texture coordinates
                // glTF supports multiple sets, we only load the first one
                if (inputPrimitive.attributes.find("TEXCOORD_0") != inputPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[inputPrimitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    texCoordsBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                if (inputPrimitive.attributes.find("COLOR_0") != inputPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[inputPrimitive.attributes.find("COLOR_0")->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    coloursBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                // Append data to model's vertex buffer
                for (size_t v = 0; v < vertexCount; v++) {
                    Vertex vert{};
                    vert.mPosition = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                    vert.mNormal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                    vert.mTexCoord0 = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
                    vert.mColor = coloursBuffer ? glm::make_vec3(&coloursBuffer[v * 3]) : glm::vec3(1.0f);
                    data.mVertices.push_back(vert);
                    mVertexCount++;
                }
            }
            // Indices
            {
                const tinygltf::Accessor& accessor = model.accessors[inputPrimitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                indexCount += static_cast<uint32_t>(accessor.count);

                // glTF supports different component types of indices
                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++) {
                        data.mIndices.push_back(buf[index] + vertexStart);
                        mIndexCount++;
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++) {
                        data.mIndices.push_back(buf[index] + vertexStart);
                        mIndexCount++;
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++) {
                        data.mIndices.push_back(buf[index] + vertexStart);
                        mIndexCount++;
                    }
                    break;
                }
                default:
                    std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                    return;
                }
            }
            glTFPrimitive primitive{};
            primitive.mFirstIndex = firstIndex;
            primitive.mIndexCount = indexCount;
            primitive.mMaterialIndex = inputPrimitive.material;
            node->mMesh.mPrimitives.push_back(primitive);
        }
    }

    if (parent)
        parent->mChildren.push_back(node);
    else
        mNodes.push_back(node);
}

void VRE::VRE_glTFModel::Bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = { mVertexBuffer->GetBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void VRE::VRE_glTFModel::Render(VkCommandBuffer commandBuffer, VkPipelineLayout& pipelineLayout, VRE_DescriptorWriter& writer)
{
    for (auto& node : mNodes)
        RenderNode(commandBuffer, node, pipelineLayout, writer);
}

void VRE::VRE_glTFModel::RenderNode(VkCommandBuffer commandBuffer, glTFNode* node, VkPipelineLayout& pipelineLayout, VRE_DescriptorWriter& writer)
{
    if (node->mMesh.mPrimitives.size() > 0) {
        for (glTFPrimitive& primitive : node->mMesh.mPrimitives) {
            if (primitive.mIndexCount > 0) {
                auto texture = mTextureIndices[mMaterials[primitive.mMaterialIndex].mBaseColorTextureIndex];
                auto imageInfo = mTextures[texture]->GetDescImageInfo();
                writer.WriteImage(1, &imageInfo);
                VkDescriptorSet gameObjectDescriptorSet;
                writer.Build(gameObjectDescriptorSet);
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &gameObjectDescriptorSet, 0, nullptr);
                vkCmdDrawIndexed(commandBuffer, primitive.mIndexCount, 1, primitive.mFirstIndex, 0, 0);
            }
        }
    }
    for (auto& child : node->mChildren) {
        RenderNode(commandBuffer, child, pipelineLayout, writer);
    }
}

void VRE::VRE_glTFModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
    mVertexCount = static_cast<uint32_t>(vertices.size());

    assert(mVertexCount >= 3 && "Vertex count must be at least 3!");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * mVertexCount;

    uint32_t vertexSize = sizeof(vertices[0]);

    VRE_Buffer stagingBuffer(mDevice, vertexSize, mVertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.Map();
    stagingBuffer.WriteToBuffer((void*)vertices.data());

    mVertexBuffer = std::make_unique<VRE_Buffer>(mDevice, vertexSize, mVertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    mDevice.CopyBuffer(stagingBuffer.GetBuffer(), mVertexBuffer->GetBuffer(), bufferSize);
}

void VRE::VRE_glTFModel::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
    mIndexCount = static_cast<uint32_t>(indices.size());

    VkDeviceSize bufferSize = sizeof(indices[0]) * mIndexCount;
    uint32_t indexSize = sizeof(indices[0]);

    VRE_Buffer stagingBuffer(mDevice, indexSize, mIndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.Map();
    stagingBuffer.WriteToBuffer((void*)indices.data());

    mIndexBuffer = std::make_unique<VRE_Buffer>(mDevice, indexSize, mIndexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    mDevice.CopyBuffer(stagingBuffer.GetBuffer(), mIndexBuffer->GetBuffer(), bufferSize);
}
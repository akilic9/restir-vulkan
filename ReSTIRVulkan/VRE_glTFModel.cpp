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

    CreateTextureSamplers(model);
    LoadTextures(model);
    LoadMaterials(model);

    const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];

    ModelData data{};

    for (size_t i = 0; i < scene.nodes.size(); i++) {
        const tinygltf::Node node = model.nodes[scene.nodes[i]];
        LoadNode(nullptr, node, scene.nodes[i], model, data);
    }

    CreateVertexBuffers(data.mVertices);
    CreateIndexBuffer(data.mIndices);
}

void VRE::VRE_glTFModel::LoadTextures(tinygltf::Model& model)
{
    for (tinygltf::Texture& tex : model.textures) {
        std::string filePath = mFileFolder;
        filePath.append(model.images[tex.source].name).append(model.images[tex.source].uri);
        if (tex.sampler == -1) {
            mTextures.push_back(std::move(VRE_Texture::CreateTexture(mDevice, filePath)));
            continue;
        }

        mTextures.push_back(std::move(VRE_Texture::CreateTexture(mDevice, filePath, mTextureSamplerProps[tex.sampler])));
    }
}

void VRE::VRE_glTFModel::LoadMaterials(tinygltf::Model& model)
{
    for (tinygltf::Material& mat : model.materials) {
        glTFMaterial material{};
        material.mDoubleSided = mat.doubleSided;
        if (mat.values.find("baseColorTexture") != mat.values.end()) {
            material.mBaseColorTexture = mTextures[mat.values["baseColorTexture"].TextureIndex()];
            material.mTexCoordSets.mBaseColor = mat.values["baseColorTexture"].TextureTexCoord();
        }
        if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
            material.mMetallicRoughnessTexture = mTextures[mat.values["metallicRoughnessTexture"].TextureIndex()];
            material.mTexCoordSets.mMetallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
        }
        if (mat.values.find("roughnessFactor") != mat.values.end()) {
            material.mRoughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
        }
        if (mat.values.find("metallicFactor") != mat.values.end()) {
            material.mMetallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
        }
        if (mat.values.find("baseColorFactor") != mat.values.end()) {
            material.mBaseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
        }
        if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
            material.mNormalTexture = mTextures[mat.additionalValues["normalTexture"].TextureIndex()];
            material.mTexCoordSets.mNormal = mat.additionalValues["normalTexture"].TextureTexCoord();
        }
        if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
            material.mEmissiveTexture = mTextures[mat.additionalValues["emissiveTexture"].TextureIndex()];
            material.mTexCoordSets.mEmissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
        }
        if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
            material.mOcclusionTexture = mTextures[mat.additionalValues["occlusionTexture"].TextureIndex()];
            material.mTexCoordSets.mOcclusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
        }
        if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
            tinygltf::Parameter param = mat.additionalValues["alphaMode"];
            if (param.string_value == "BLEND") {
                material.mAlphaMode = glTFMaterial::ALPHAMODE_BLEND;
            }
            if (param.string_value == "MASK") {
                material.mAlphaCutoff = 0.5f;
                material.mAlphaMode = glTFMaterial::ALPHAMODE_MASK;
            }
        }
        if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
            material.mAlphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
        }
        if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
            material.mEmissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
        }

        // Extensions
        if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) {
            auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
            if (ext->second.Has("specularGlossinessTexture")) {
                auto index = ext->second.Get("specularGlossinessTexture").Get("index");
                material.mExtension.mSpecularGlossTexture = mTextures[index.Get<int>()];
                auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
                material.mTexCoordSets.mSpecularGloss = texCoordSet.Get<int>();
                material.mPbrWorkflows.mSpecularGloss = true;
                material.mPbrWorkflows.mMetallicRoughness = false;
            }
            if (ext->second.Has("diffuseTexture")) {
                auto index = ext->second.Get("diffuseTexture").Get("index");
                material.mExtension.mDiffuseTexture = mTextures[index.Get<int>()];
            }
            if (ext->second.Has("diffuseFactor")) {
                auto factor = ext->second.Get("diffuseFactor");
                for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
                    auto val = factor.Get(i);
                    material.mExtension.mDiffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
                }
            }
            if (ext->second.Has("specularFactor")) {
                auto factor = ext->second.Get("specularFactor");
                for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
                    auto val = factor.Get(i);
                    material.mExtension.mSpecularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
                }
            }
        }

        if (mat.extensions.find("KHR_materials_unlit") != mat.extensions.end()) {
            material.mUnlit = true;
        }

        if (mat.extensions.find("KHR_materials_emissive_strength") != mat.extensions.end()) {
            auto ext = mat.extensions.find("KHR_materials_emissive_strength");
            if (ext->second.Has("emissiveStrength")) {
                auto value = ext->second.Get("emissiveStrength");
                material.mEmissiveStrength = (float)value.Get<double>();
            }
        }

        material.mIndex = static_cast<uint32_t>(mMaterials.size());
        mMaterials.push_back(material);
    }

    // Push a default material at the end of the list for meshes with no material assigned
    mMaterials.push_back(glTFMaterial());
}

void VRE::VRE_glTFModel::LoadNode(std::shared_ptr<glTFNode> parent, const tinygltf::Node& inputNode, uint32_t nodeIndex, const tinygltf::Model& model, ModelData& data)
{
    std::shared_ptr<glTFNode> node = std::make_shared<glTFNode>(nodeIndex, inputNode.name, glm::mat4(1.0f), parent);
    node->mIndex = nodeIndex;
    node->mParent = parent;
    node->mName = inputNode.name;
    node->mMatrix = glm::mat4(1.0f);
    
    glm::vec3 translation = glm::vec3(0.0f);
    if (inputNode.translation.size() == 3) {
        translation = glm::make_vec3(inputNode.translation.data());
        node->mTranslation = translation;
    }

    glm::mat4 rotation = glm::mat4(1.0f);
    if (inputNode.rotation.size() == 4) {
        glm::quat q = glm::make_quat(inputNode.rotation.data());
        node->mRotation = glm::mat4(q);
    }

    glm::vec3 scale = glm::vec3(1.0f);
    if (inputNode.scale.size() == 3) {
        scale = glm::make_vec3(inputNode.scale.data());
        node->mScale = scale;
    }

    if (inputNode.matrix.size() == 16)
        node->mMatrix = glm::make_mat4x4(inputNode.matrix.data());

    // Node with children
    if (inputNode.children.size() > 0)
        for (size_t i = 0; i < inputNode.children.size(); i++)
            LoadNode(node, model.nodes[inputNode.children[i]], inputNode.children[i], model, data);


    if (inputNode.mesh > -1) {
        const tinygltf::Mesh inputMesh = model.meshes[inputNode.mesh];
        for (size_t i = 0; i < inputMesh.primitives.size(); i++) {
            const tinygltf::Primitive& inputPrimitive = inputMesh.primitives[i];
            uint32_t vertexStart = static_cast<uint32_t>(data.mVertexPosition);
            uint32_t indexStart = static_cast<uint32_t>(data.mIndexPosition);
            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;
            glm::vec3 posMin{};
            glm::vec3 posMax{};
            bool hasIndexBuffer = inputPrimitive.indices > -1;

            const float* bufferPos = nullptr;
            const float* bufferNormals = nullptr;
            const float* bufferTexCoordSet0 = nullptr;
            const float* bufferTexCoordSet1 = nullptr;
            const float* bufferColorSet = nullptr;

            int posByteStride;
            int normByteStride;
            int texCoord0ByteStride;
            int texCoord1ByteStride;
            int color0ByteStride;

            // Position attribute is required
            assert(inputPrimitive.attributes.find("POSITION") != inputPrimitive.attributes.end());

            const tinygltf::Accessor& posAccessor = model.accessors[inputPrimitive.attributes.find("POSITION")->second];
            const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
            bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
            posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
            posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
            vertexCount = static_cast<uint32_t>(posAccessor.count);
            posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

            if (inputPrimitive.attributes.find("NORMAL") != inputPrimitive.attributes.end()) {
                const tinygltf::Accessor& normAccessor = model.accessors[inputPrimitive.attributes.find("NORMAL")->second];
                const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
                bufferNormals = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
                normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
            }

            // UVs
            if (inputPrimitive.attributes.find("TEXCOORD_0") != inputPrimitive.attributes.end()) {
                const tinygltf::Accessor& uvAccessor = model.accessors[inputPrimitive.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
                bufferTexCoordSet0 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
                texCoord0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
            }
            if (inputPrimitive.attributes.find("TEXCOORD_1") != inputPrimitive.attributes.end()) {
                const tinygltf::Accessor& uvAccessor = model.accessors[inputPrimitive.attributes.find("TEXCOORD_1")->second];
                const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
                bufferTexCoordSet1 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
                texCoord1ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
            }

            // Vertex colors
            if (inputPrimitive.attributes.find("COLOR_0") != inputPrimitive.attributes.end()) {
                const tinygltf::Accessor& accessor = model.accessors[inputPrimitive.attributes.find("COLOR_0")->second];
                const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                bufferColorSet = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                color0ByteStride = accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
            }

            for (size_t v = 0; v < posAccessor.count; v++) {
                Vertex vertex{};
                vertex.mPosition = glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);
                vertex.mNormal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
                vertex.mTexCoord0 = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * texCoord0ByteStride]) : glm::vec3(0.0f);
                vertex.mTexCoord1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * texCoord1ByteStride]) : glm::vec3(0.0f);
                vertex.mColor = bufferColorSet ? glm::make_vec4(&bufferColorSet[v * color0ByteStride]) : glm::vec4(1.0f);
                data.mVertices.push_back(vertex);
                data.mVertexPosition++;
            }

            if (!hasIndexBuffer)
                continue;

            const tinygltf::Accessor& accessor = model.accessors[inputPrimitive.indices > -1 ? inputPrimitive.indices : 0];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

            indexCount = static_cast<uint32_t>(accessor.count);
            const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

            switch (accessor.componentType) {
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
                for (size_t index = 0; index < accessor.count; index++) {
                    data.mIndices.push_back(buf[index] + vertexStart);
                    data.mIndexPosition++;
                }
                break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
                for (size_t index = 0; index < accessor.count; index++) {
                    data.mIndices.push_back(buf[index] + vertexStart);
                    data.mIndexPosition++;
                }
                break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
                for (size_t index = 0; index < accessor.count; index++) {
                    data.mIndices.push_back(buf[index] + vertexStart);
                    data.mIndexPosition++;
                }
                break;
            }
            default:
                std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                return;
            }

            std::unique_ptr<glTFPrimitive> primitive = std::make_unique<glTFPrimitive>(indexStart, indexCount, vertexCount, inputPrimitive.material > -1 ? mMaterials[inputPrimitive.material] : mMaterials.back());
            node->mMesh->mPrimitives.push_back(std::move(primitive));
        }
    }
    if (parent)
        parent->mChildren.push_back(node);
    else
        mNodes.push_back(node);

    mAllNodes.push_back(node);
}

void VRE::VRE_glTFModel::CreateTextureSamplers(tinygltf::Model& model)
{
    for (tinygltf::Sampler sampler : model.samplers) {
        VRE_Texture::SamplerProperties sp{};
        sp.minFilter = GetFilterMode(sampler.minFilter);
        sp.magFilter = GetFilterMode(sampler.magFilter);
        sp.addressModeU = GetWrapMode(sampler.wrapS);
        sp.addressModeV = GetWrapMode(sampler.wrapT);
        sp.addressModeW = sp.addressModeV;
        mTextureSamplerProps.push_back(sp);
    }
}

void VRE::VRE_glTFModel::Draw(VkCommandBuffer commandBuffer)
{
    for (auto &node : mNodes)
        DrawNode(commandBuffer, node);
}

void VRE::VRE_glTFModel::DrawNode(VkCommandBuffer commandBuffer, std::shared_ptr<glTFNode> node)
{
    if (node->mMesh) {
        for (auto& primitive : node->mMesh->mPrimitives) {
            vkCmdDrawIndexed(commandBuffer, primitive->mIndexCount, 1, primitive->mFirstIndex, 0, 0);
        }
    }
    for (auto& child : node->mChildren) {
        DrawNode(commandBuffer, child);
    }
}

void VRE::VRE_glTFModel::Bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = { mVertexBuffer->GetBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void VRE::VRE_glTFModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
    auto vertexCount = static_cast<uint32_t>(vertices.size());

    assert(vertexCount >= 3 && "Vertex count must be at least 3!");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

    uint32_t vertexSize = sizeof(vertices[0]);

    VRE_Buffer stagingBuffer(mDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.Map();
    stagingBuffer.WriteToBuffer((void*)vertices.data());

    mVertexBuffer = std::make_unique<VRE_Buffer>(mDevice, vertexSize, vertexCount,
                                                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    mDevice.CopyBuffer(stagingBuffer.GetBuffer(), mVertexBuffer->GetBuffer(), bufferSize);
}

void VRE::VRE_glTFModel::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
    auto indexCount = static_cast<uint32_t>(indices.size());

    VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
    uint32_t indexSize = sizeof(indices[0]);

    VRE_Buffer stagingBuffer(mDevice, indexSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.Map();
    stagingBuffer.WriteToBuffer((void*)indices.data());

    mIndexBuffer = std::make_unique<VRE_Buffer>(mDevice, indexSize, indexCount,
                                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    mDevice.CopyBuffer(stagingBuffer.GetBuffer(), mIndexBuffer->GetBuffer(), bufferSize);
}

VkSamplerAddressMode VRE::VRE_glTFModel::GetWrapMode(int32_t wrapMode)
{
    switch (wrapMode) {
    case -1:
    case 10497:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case 33071:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case 33648:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }

    std::cerr << "Unknown wrap mode for getVkWrapMode: " << wrapMode << std::endl;
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

VkFilter VRE::VRE_glTFModel::GetFilterMode(int32_t filterMode)
{
    switch (filterMode) {
    case -1:
    case 9728:
        return VK_FILTER_NEAREST;
    case 9729:
        return VK_FILTER_LINEAR;
    case 9984:
        return VK_FILTER_NEAREST;
    case 9985:
        return VK_FILTER_NEAREST;
    case 9986:
        return VK_FILTER_LINEAR;
    case 9987:
        return VK_FILTER_LINEAR;
    }

    std::cerr << "Unknown filter mode for getVkFilterMode: " << filterMode << std::endl;
    return VK_FILTER_NEAREST;
}
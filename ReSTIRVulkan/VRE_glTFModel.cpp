#include "VRE_glTFModel.h"
#include <iostream>
#include <gtc/type_ptr.hpp>

// TinyGLTF library definitions and include.
// Example and instructions: https://github.com/syoyo/tinygltf?tab=readme-ov-file#loading-gltf-20-model
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

VRE::VRE_glTFModel::VRE_glTFModel(VRE_Device& device, const std::string& fileFolder, const std::string& fileName)
    : mDevice(device)
    , mVertexCount(0)
    , mIndexCount(0)
    , mFileFolder(fileFolder)
    , mFileName(fileName)
{

}

VRE::VRE_glTFModel::~VRE_glTFModel()
{
    for (auto node : mNodes) delete node;
}

// Just trying to test whether LoadNode and other stuff is working for now, so threw everything in this function.
// The class will be cleaned up and organized once I get the code working.
void VRE::VRE_glTFModel::LoadImages()
{
    tinygltf::Model glTFInput;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;

    std::string filePath = mFileFolder;

    if (!gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filePath.append(mFileName).append(".gltf")))
        throw std::runtime_error("Failed to load glTF file!");

    // Load textures
    mTextures.resize(glTFInput.images.size());
    filePath = mFileFolder;

    for (auto &image : glTFInput.images) {
        std::cout << filePath.append(image.name).append(image.uri) << std::endl;
        mTextures.push_back(std::move(VRE_Texture::CreateTexture(mDevice, filePath)));
    }

    // Load materials
    mMaterials.resize(glTFInput.materials.size());
    for (size_t i = 0; i < glTFInput.materials.size(); i++) {
        // We only read the most basic properties required for our sample
        tinygltf::Material glTFMaterial = glTFInput.materials[i];
        // Get the base color factor
        if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
            mMaterials[i].mBaseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
        }
        // Get base color texture index
        if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
            mMaterials[i].mBaseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
        }
    }

    // Load texture indices
    mTextureIndices.resize(glTFInput.textures.size());
    for (size_t i = 0; i < glTFInput.textures.size(); i++) {
        mTextureIndices[i] = glTFInput.textures[i].source;
    }

    const tinygltf::Scene& scene = glTFInput.scenes[0];
    for (size_t i = 0; i < scene.nodes.size(); i++) {
        const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
        LoadNode(node, glTFInput, nullptr);
    }


}

// Want to pass tinygltf::Node and tinygltf::Model here, but how should the include be done in this case?
void VRE::VRE_glTFModel::LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent)
{
    Node* node = new Node{};
    node->mMatrix = glm::mat4(1.0f);
    node->mParent = parent;

    // Get the local node matrix
    // It's either made up from translation, rotation, scale or a 4x4 matrix
    if (inputNode.translation.size() == 3) {
        node->mMatrix = glm::translate(node->mMatrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
    }
    if (inputNode.rotation.size() == 4) {
        glm::quat q = glm::make_quat(inputNode.rotation.data());
        node->mMatrix *= glm::mat4(q);
    }
    if (inputNode.scale.size() == 3) {
        node->mMatrix = glm::scale(node->mMatrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
    }
    if (inputNode.matrix.size() == 16) {
        node->mMatrix = glm::make_mat4x4(inputNode.matrix.data());
    };

    // Load node's children
    if (inputNode.children.size() > 0) {
        for (size_t i = 0; i < inputNode.children.size(); i++) {
            LoadNode(input.nodes[inputNode.children[i]], input, node);
        }
    }

    // If the node contains mesh data, we load vertices and indices from the buffers
    // In glTF this is done via accessors and buffer views
    if (inputNode.mesh > -1) {
        const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];

        // Iterate through all primitives of this node's mesh
        for (size_t i = 0; i < mesh.primitives.size(); i++) {
            const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
            uint32_t firstIndex = static_cast<uint32_t>(mIndexCount);
            uint32_t vertexStart = static_cast<uint32_t>(mVertexCount);
            uint32_t indexCount = 0;

            // Vertices
            {
                const float* positionBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;
                size_t vertexCount = 0;

                // Get buffer data for vertex positions
                if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    vertexCount = accessor.count;
                }

                // Get buffer data for vertex normals
                if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                // Get buffer data for vertex texture coordinates
                // glTF supports multiple sets, we only load the first one
                if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                // Append data to model's vertex buffer
                for (size_t v = 0; v < vertexCount; v++) {
                    Vertex vert{};
                    vert.mPosition = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                    vert.mNormal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                    vert.mTexCoord = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
                    vert.mColor = glm::vec3(1.0f);
                    mModelData.mVertices.push_back(vert);
                    mVertexCount++;
                }
            }
            // Indices
            {
                const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
                const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

                indexCount += static_cast<uint32_t>(accessor.count);

                // glTF supports different component types of indices
                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++) {
                        mModelData.mIndices.push_back(buf[index] + vertexStart);
                        mIndexCount++;
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++) {
                        mModelData.mIndices.push_back(buf[index] + vertexStart);
                        mIndexCount++;
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++) {
                        mModelData.mIndices.push_back(buf[index] + vertexStart);
                        mIndexCount++;
                    }
                    break;
                }
                default:
                    std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                    return;
                }
            }
            Primitive primitive{};
            primitive.mFirstIndex = firstIndex;
            primitive.mIndexCount = indexCount;
            primitive.mMaterialIndex = glTFPrimitive.material;
            node->mMesh.mPrimitives.push_back(primitive);
        }
    }

    if (parent)
        parent->mChildren.push_back(node);
    else
        mNodes.push_back(node);
}

void VRE::VRE_glTFModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
}

void VRE::VRE_glTFModel::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
}

void VRE::VRE_glTFModel::Bind(VkCommandBuffer commandBuffer)
{
}

void VRE::VRE_glTFModel::Draw(VkCommandBuffer commandBuffer)
{
}
#pragma once
#include "VRE_Device.h"
#include "VRE_Buffer.h"
#include "VRE_Texture.h"

#include <vector>
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

namespace VRE {
	class VRE_glTFModel {
    public:
        struct Vertex {
            glm::vec3 mPosition;
            glm::vec3 mColor;
            glm::vec3 mNormal;
            glm::vec2 mTexCoord;

            static std::vector<VkVertexInputBindingDescription> GetBindingDesc();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDesc();

            bool operator==(const Vertex& other) const {
                return mPosition == other.mPosition && mColor == other.mColor &&
                    mNormal == other.mNormal && mTexCoord == other.mTexCoord;
            }
        };

        struct ModelData {
            std::vector<Vertex> mVertices;
            std::vector<uint32_t> mIndices;
        };

        //Forward delare Node.
        struct Node;

        struct Primitive {
            uint32_t mFirstIndex;
            uint32_t mIndexCount;
            int32_t mMaterialIndex;
        };

        // Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives.
        struct Mesh {
            std::vector<Primitive> mPrimitives;
        };

        // A node represents an object in the glTF scene graph.
        struct Node {
            Node* mParent;
            std::vector<Node*> mChildren;
            Mesh mMesh;
            glm::mat4 mMatrix;
            ~Node() { for (auto& child : mChildren) delete child; }
        };

        // A glTF material stores information in e.g. the texture that is attached to it and colors.
        struct Material {
            glm::vec4 mBaseColorFactor = glm::vec4(1.0f);
            uint32_t mBaseColorTextureIndex;
        };

        VRE_glTFModel(VRE_Device& device, const std::string& fileFolder, const std::string& fileName);
        ~VRE_glTFModel();

        VRE_glTFModel(const VRE_glTFModel&) = delete;
        VRE_glTFModel& operator=(const VRE_glTFModel&) = delete;

        static std::unique_ptr<VRE_glTFModel> CreateModel(VRE_Device& device, const std::string& filePath);

        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);

        void LoadImages();
        void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent);

    private:
        void CreateVertexBuffers(const std::vector<Vertex>& vertices);
        void CreateIndexBuffer(const std::vector<uint32_t>& indices);

        VRE_Device& mDevice;

        std::unique_ptr<VRE_Buffer> mVertexBuffer;
        uint32_t mVertexCount;

        std::unique_ptr<VRE_Buffer> mIndexBuffer;
        uint32_t mIndexCount;

        std::vector<std::unique_ptr<VRE_Texture>> mTextures;
        std::vector<Material> mMaterials;
        std::vector<Node*> mNodes;
        std::vector<int32_t> mTextureIndices;

        const std::string mFileFolder;
        const std::string mFileName;

        ModelData mModelData;
    };
}
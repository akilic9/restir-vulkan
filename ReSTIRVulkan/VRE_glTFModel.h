#pragma once
#include "VRE_Device.h"
#include "VRE_Buffer.h"
#include "VRE_Texture.h"
#include "VRE_ModelProperties.h"
#include "VRE_Pipeline.h"
#include "VRE_Descriptor.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

namespace VRE {
	class VRE_glTFModel {
    public:
        //Forward declare Node.
        struct VRE_Node;

        struct VRE_Primitive {
            uint32_t mFirstIndex;
            uint32_t mIndexCount;
            int32_t mMaterialIndex;
        };

        // Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives.
        struct VRE_Mesh {
            std::vector<VRE_Primitive> mPrimitives;
        };

        // A node represents an object in the glTF scene graph.
        struct VRE_Node {
            VRE_Node* mParent;
            std::vector<VRE_Node*> mChildren;
            VRE_Mesh mMesh;
            ~VRE_Node() { for (auto& child : mChildren) delete child; }
        };

        // A glTF material stores information in e.g. the texture that is attached to it and colors.
        struct VRE_Material {
            glm::vec4 mBaseColorFactor = glm::vec4(1.0f);
            uint32_t mBaseColorTextureIndex;
        };

        VRE_glTFModel(VRE_Device& device, const std::string& fileFolder, const std::string& fileName);
        ~VRE_glTFModel();

        VRE_glTFModel(const VRE_glTFModel&) = delete;
        VRE_glTFModel& operator=(const VRE_glTFModel&) = delete;

        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout& pipelineLayout, VRE_DescriptorWriter& writer);
        void DrawNode(VkCommandBuffer commandBuffer, VRE_Node* node, VkPipelineLayout& pipelineLayout, VRE_DescriptorWriter& writer);

        void LoadImages();
        void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, VRE_Node* parent);

    private:
        void CreateVertexBuffers(const std::vector<Vertex>& vertices);
        void CreateIndexBuffer(const std::vector<uint32_t>& indices);

        VRE_Device& mDevice;

        std::unique_ptr<VRE_Buffer> mVertexBuffer;
        uint32_t mVertexCount;

        std::unique_ptr<VRE_Buffer> mIndexBuffer;
        uint32_t mIndexCount;

        std::vector<std::unique_ptr<VRE_Texture>> mTextures;
        std::vector<VRE_Material> mMaterials;
        std::vector<VRE_Node*> mNodes;
        std::vector<int32_t> mTextureIndices;

        const std::string mFileFolder;
        const std::string mFileName;

        ModelData mModelData; // TODO: Might not need all this stuff to be member variable?
    };
}
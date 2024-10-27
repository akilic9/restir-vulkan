#pragma once
#include "VRE_Device.h"
#include "VRE_Buffer.h"
#include "VRE_ModelProperties.h"
#include "VRE_Pipeline.h"
#include "VRE_Descriptor.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

namespace VRE {
	class VRE_glTFModel {
    public:
        VRE_glTFModel(VRE_Device& device, const std::string& fileFolder, const std::string& fileName);
        ~VRE_glTFModel();

        VRE_glTFModel(const VRE_glTFModel&) = delete;
        VRE_glTFModel& operator=(const VRE_glTFModel&) = delete;

        void LoadModel();

        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);
        void DrawNode(VkCommandBuffer commandBuffer, std::shared_ptr<glTFNode> node);

        inline std::vector<glTFMaterial>& GetMaterials() { return mMaterials; }
        inline std::vector<std::shared_ptr<glTFNode>>& GetAllNodes() { return mAllNodes; }
        inline std::vector<std::shared_ptr<glTFNode>>& GetNodes() { return mNodes; }

    private:
        void LoadTextures(tinygltf::Model& model);
        void LoadMaterials(tinygltf::Model& model);
        void LoadNode(std::shared_ptr<glTFNode> parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, ModelData& data);

        void CreateVertexBuffers(const std::vector<Vertex>& vertices);
        void CreateIndexBuffer(const std::vector<uint32_t>& indices);

        void CreateTextureSamplers(tinygltf::Model& model);
        VkSamplerAddressMode GetWrapMode(int32_t wrapMode);
        VkFilter GetFilterMode(int32_t filterMode);

        VRE_Device& mDevice;

        const std::string mFileFolder;
        const std::string mFileName;

        std::vector<VRE_Texture::SamplerProperties> mTextureSamplerProps;
        std::vector<std::shared_ptr<VRE_Texture>> mTextures;
        std::vector<std::shared_ptr<glTFNode>> mNodes;
        std::vector<std::shared_ptr<glTFNode>> mAllNodes;
        std::vector<glTFMaterial> mMaterials;

        std::unique_ptr<VRE_Buffer> mVertexBuffer;
        std::unique_ptr<VRE_Buffer> mIndexBuffer;
    };
}
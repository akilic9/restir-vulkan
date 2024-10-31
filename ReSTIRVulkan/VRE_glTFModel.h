/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*/
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
        void Render(VkCommandBuffer commandBuffer, VkPipelineLayout& pipelineLayout, VRE_DescriptorWriter& writer);
        void RenderNode(VkCommandBuffer commandBuffer, glTFNode* node, VkPipelineLayout& pipelineLayout, VRE_DescriptorWriter& writer);

    private:
        void LoadTextures(tinygltf::Model& model);
        void LoadMaterials(tinygltf::Model& model);
        void LoadNode(glTFNode* parent, const tinygltf::Node& node, const tinygltf::Model& model, ModelData& data);

        void CreateVertexBuffers(const std::vector<Vertex>& vertices);
        void CreateIndexBuffer(const std::vector<uint32_t>& indices);

        VRE_Device& mDevice;

        const std::string mFileFolder;
        const std::string mFileName;

        std::vector<std::shared_ptr<VRE_Texture>> mTextures;
        std::vector<glTFNode*> mNodes;
        std::vector<glTFMaterial> mMaterials;
        std::vector<int32_t> mTextureIndices;

        std::unique_ptr<VRE_Buffer> mVertexBuffer;
        std::unique_ptr<VRE_Buffer> mIndexBuffer;

        uint32_t mVertexCount;
        uint32_t mIndexCount;
    };
}
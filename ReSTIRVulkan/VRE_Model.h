#pragma once
#include "VRE_Device.h"
#include "VRE_Buffer.h"
#include "VRE_ModelProperties.h"
#include <memory>

namespace VRE {
    class VRE_Model
    {
    public:
        VRE_Model(VRE_Device& device, const std::string& filePath);
        ~VRE_Model();

        VRE_Model(const VRE_Model&) = delete;
        VRE_Model& operator=(const VRE_Model&) = delete;

        static std::unique_ptr<VRE_Model> CreateModel(VRE_Device& device, const std::string& filePath);

        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);

    private:
        void CreateVertexBuffers(const std::vector<Vertex>& vertices);
        void CreateIndexBuffer(const std::vector<uint32_t>& indices);
        void LoadModel(const std::string& filePath, ModelData& data);

        VRE_Device &mDevice;

        std::unique_ptr<VRE_Buffer> mVertexBuffer;
        uint32_t mVertexCount;

        bool mHasIndexBuffer;
        std::unique_ptr<VRE_Buffer> mIndexBuffer;
        uint32_t mIndexCount;
    };
}
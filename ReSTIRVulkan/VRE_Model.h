#pragma once
#include "VRE_Device.h"
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

namespace VRE {
    class VRE_Model
    {
    public:
        struct Vertex {
            glm::vec3 position;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> GetBindingDesc();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDesc();
        };

        struct ModelData {
            std::vector<Vertex> mVertices{};
            std::vector<uint32_t> mIndices{};
        };

        VRE_Model(VRE_Device& device, const ModelData &data);
        ~VRE_Model();

        VRE_Model(const VRE_Model&) = delete;
        VRE_Model& operator=(const VRE_Model&) = delete;

        void Bind(VkCommandBuffer commandBuffer);
        void Draw(VkCommandBuffer commandBuffer);

    private:
        void CreateVertexBuffers(const std::vector<Vertex>& vertices);
        void CreateIndexBuffer(const std::vector<uint32_t>& indices);

        VRE_Device &mDevice;

        VkBuffer mVertexBuffer;
        VkDeviceMemory mVertexBufferMemory;
        uint32_t mVertexCount;

        bool mHasIndexBuffer;
        VkBuffer mIndexBuffer;
        VkDeviceMemory mIndexBufferMemory;
        uint32_t mIndexCount;
    };
}


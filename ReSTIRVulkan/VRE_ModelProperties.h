#pragma once

#include "vulkan/vulkan.h"
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

namespace VRE {
    struct Vertex {
        glm::vec3 mPosition;
        glm::vec3 mColor;
        glm::vec3 mNormal;
        glm::vec2 mTexCoord;

        static std::vector<VkVertexInputBindingDescription> GetBindingDesc() {
            std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
            bindingDescriptions[0].binding = 0;
            bindingDescriptions[0].stride = sizeof(Vertex);
            bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return bindingDescriptions;
        }

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDesc() {
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

            attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, mPosition) });
            attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, mColor) });
            attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, mNormal) });
            attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, mTexCoord) });

            return attributeDescriptions;
        }

        bool operator==(const Vertex& other) const {
            return mPosition == other.mPosition && mColor == other.mColor &&
                mNormal == other.mNormal && mTexCoord == other.mTexCoord;
        }
    };

    struct ModelData {
        std::vector<Vertex> mVertices;
        std::vector<uint32_t> mIndices;
    };
}

#include "VRE_Utilities.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>

namespace std {
    template <>
    struct hash<VRE::Vertex> {
        size_t operator()(VRE::Vertex const& vertex) const {
            size_t seed = 0;
            VRE::hashCombine(seed, vertex.mPosition, vertex.mColor, vertex.mNormal, vertex.mTexCoord);
            return seed;
        }
    };
}  // namespace std

/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*/
#pragma once

#include "vulkan/vulkan.h"
#include "VRE_Texture.h"
#include "VRE_Buffer.h"
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/quaternion.hpp>

namespace VRE {
    struct Vertex {
        glm::vec3 mPosition;
        glm::vec3 mColor;
        glm::vec3 mNormal;
        glm::vec2 mTexCoord0;

        static std::vector<VkVertexInputBindingDescription> GetBindingDesc();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDesc();
        bool operator==(const Vertex& other) const;
    };

    struct ModelData {
        std::vector<Vertex> mVertices;
        std::vector<uint32_t> mIndices;
    };

    struct glTFMaterial {
        glm::vec4 mBaseColorFactor = glm::vec4(1.0f);
        uint32_t mBaseColorTextureIndex;
    };

    struct glTFPrimitive {
        uint32_t mFirstIndex;
        uint32_t mIndexCount;
        int32_t mMaterialIndex;
    };

    struct glTFMesh {
        std::vector<glTFPrimitive> mPrimitives;
    };

    struct glTFNode {
        glTFNode* mParent;
        std::vector<glTFNode*> mChildren;
        glTFMesh mMesh;
        ~glTFNode() { for (auto& child : mChildren) delete child; }
    };
}
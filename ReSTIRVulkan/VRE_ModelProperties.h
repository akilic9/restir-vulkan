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
        //glm::vec2 mTexCoord1;

        static std::vector<VkVertexInputBindingDescription> GetBindingDesc();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDesc();
        bool operator==(const Vertex& other) const;
    };

    struct ModelData {
        std::vector<Vertex> mVertices;
        std::vector<uint32_t> mIndices;
        size_t mIndexPosition = 0;
        size_t mVertexPosition = 0;
    };

    struct glTFMaterial {
        enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
        AlphaMode mAlphaMode = ALPHAMODE_OPAQUE;

        int mIndex = 0;
        bool mUnlit = false;
        bool mDoubleSided = false;

        float mAlphaCutoff = 1.0f;
        float mMetallicFactor = 1.0f;
        float mRoughnessFactor = 1.0f;
        float mEmissiveStrength = 1.0f;

        glm::vec4 mBaseColorFactor = glm::vec4(1.0f);
        glm::vec4 mEmissiveFactor = glm::vec4(0.0f);

        VkDescriptorSet mDescriptor{};

        std::shared_ptr<VRE_Texture> mBaseColorTexture;
        std::shared_ptr<VRE_Texture> mMetallicRoughnessTexture;
        std::shared_ptr<VRE_Texture> mNormalTexture;
        std::shared_ptr<VRE_Texture> mOcclusionTexture;
        std::shared_ptr<VRE_Texture> mEmissiveTexture;

        struct TexCoordSets {
            uint8_t mBaseColor = 0;
            uint8_t mMetallicRoughness = 0;
            uint8_t mSpecularGloss = 0;
            uint8_t mNormal = 0;
            uint8_t mOcclusion = 0;
            uint8_t mEmissive = 0;
        } mTexCoordSets;

        struct Extension {
            glm::vec4 mDiffuseFactor = glm::vec4(1.0f);
            glm::vec3 mSpecularFactor = glm::vec3(0.0f);
            std::shared_ptr<VRE_Texture> mSpecularGlossTexture;
            std::shared_ptr<VRE_Texture> mDiffuseTexture;
        } mExtension;

        struct PbrWorkflows {
            bool mMetallicRoughness = true;
            bool mSpecularGloss = false;
        } mPbrWorkflows;
    };

    struct glTFPrimitive {
        uint32_t mFirstIndex;
        uint32_t mIndexCount;
        uint32_t mVertexCount;
        glTFMaterial& mMaterial;
        bool mHasIndices;
        glTFPrimitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, glTFMaterial& material);
    };

    struct glTFMesh {
        std::vector<std::unique_ptr<glTFPrimitive>> mPrimitives;
        glm::mat4 mMatrix;
        std::unique_ptr<VRE_Buffer> mBuffer;
        VkDescriptorSet mDescriptor{};
        glTFMesh(VRE_Device& device, glm::mat4 matrix);
        ~glTFMesh();
    };

    struct glTFNode {
        uint32_t mIndex;
        std::string mName;
        glm::mat4 mMatrix;
        glm::vec3 mTranslation{ 0.f };
        glm::vec3 mScale{ 1.0f };
        glm::quat mRotation{ };
        glm::mat4 mCachedMatrix{ glm::mat4(1.0f) };
        glm::mat4 mCachedLocalMatrix{ glm::mat4(1.0f) };
        bool mUseCachedMatrix = false;

        std::unique_ptr<glTFMesh> mMesh;
        std::shared_ptr<glTFNode> mParent;
        std::vector<std::shared_ptr<glTFNode>> mChildren;

        glTFNode(VRE_Device& device, uint32_t index, std::string name, glm::mat4 matrix, std::shared_ptr<glTFNode> parent);
        glm::mat4 GetLocalMatrix();
        glm::mat4 GetMatrix();
    };
}
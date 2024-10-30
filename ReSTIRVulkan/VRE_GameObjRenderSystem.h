#pragma once
#include "VRE_Device.h"
#include "VRE_GameObject.h"
#include "VRE_Pipeline.h"
#include "VRE_SharedContext.h"
#include <memory>

namespace VRE {
    class VRE_GameObjRenderSystem
    {
    public:
        enum PipelineType {
            PBR = 0,
            PBR_DoubleSide,
            PBR_AplhaBlend,
            Unlit,
            Unlit_DoubleSide,
            Unlit_AlphaBlend
        };

        struct DescSetLayouts {
            std::unique_ptr<VRE_DescriptorSetLayout> mMaterialDescSetLayout;
            std::unique_ptr<VRE_DescriptorSetLayout> mNodeDescSetLayout;
            std::unique_ptr<VRE_DescriptorSetLayout> mMaterialBufferDescSetLayout;
        };

        struct alignas(16) ShaderMaterial {
            glm::vec4 mBaseColorFactor;
            glm::vec4 mEmissiveFactor;
            glm::vec4 mDiffuseFactor;
            glm::vec4 mSpecularFactor;
            float mWorkflow;
            int mBaseColorTextureSet;
            int mPhysicalDescriptorTextureSet;
            int mNormalTextureSet;
            int mOcclusionTextureSet;
            int mEmissiveTextureSet;
            float mMetallicFactor;
            float mRoughnessFactor;
            float mAlphaMask;
            float mAlphaCutoff;
            float mEmissiveStrength;
        };

        using PipelinesMap = std::unordered_map<PipelineType, std::unique_ptr<VRE_Pipeline>>;

        VRE_GameObjRenderSystem(VRE_SharedContext* sharedContext);
        ~VRE_GameObjRenderSystem();

        VRE_GameObjRenderSystem(const VRE_GameObjRenderSystem&) = delete;
        VRE_GameObjRenderSystem& operator=(const VRE_GameObjRenderSystem&) = delete;

        void Init();
        void Render();

    private:
        void CreatePipelineLayouts();
        void CreatePipelines();
        void CreateDescSetLayouts();
        void CreateDescPools();
        //void CreateShaderMatBuffer();
        //void WriteToMaterialDesc();
        //void WriteToNodeDesc();
        //void WriteToNodeDescByNode(std::shared_ptr<glTFNode> node);
        //void WriteToMaterialBufferDesc();
        void RenderNode(std::shared_ptr<glTFNode> node);

        //PipelinesMap mPipelines;
        std::unique_ptr<VRE_Pipeline> mPipeline;
        VkPipelineLayout mPipelineLayout;
        std::unique_ptr<VRE_DescriptorPool> mDescPool;
        VRE_SharedContext* mSharedContext;
        std::unique_ptr<VRE_DescriptorSetLayout> mDescSetLayout;
        //DescSetLayouts mDescSetLayouts;
        //std::unique_ptr<VRE_Buffer> mShaderMatBuffer;
        //VkDescriptorSet mMatBufferDescriptor;
        //std::unique_ptr<VRE_Texture> mEmptyTexture;
    };
}
#include "VRE_GameObjRenderSystem.h"
#include <stdexcept>

VRE::VRE_GameObjRenderSystem::VRE_GameObjRenderSystem(VRE_SharedContext* sharedContext)
    : mSharedContext(sharedContext) {}

VRE::VRE_GameObjRenderSystem::~VRE_GameObjRenderSystem()
{
    vkDestroyPipelineLayout(mSharedContext->mDevice->GetVkDevice(), mPipelineLayout, nullptr);
}

void VRE::VRE_GameObjRenderSystem::Init()
{
    mEmptyTexture = VRE_Texture::CreateTexture(*mSharedContext->mDevice, "Resources/Textures/empty.png");
    CreatePipelineLayouts();
    CreatePipelines();
    CreateShaderMatBuffer();
    WriteToMaterialDesc();
    WriteToNodeDesc();
    WriteToMaterialBufferDesc();
}

void VRE::VRE_GameObjRenderSystem::Render()
{
    for (auto& e : *mSharedContext->mGameObjMap) {
        GameObjectBufferData data{};
        data.mModelMatrix = e.second.mTransform.Mat4();
        data.mModelMatrix = e.second.mTransform.NormalMatrix();

        for (auto& node : e.second.mModel->GetNodes()) {
            RenderNode(node, glTFMaterial::AlphaMode::ALPHAMODE_OPAQUE, data);
        }
        for (auto& node : e.second.mModel->GetNodes()) {
            RenderNode(node, glTFMaterial::AlphaMode::ALPHAMODE_MASK, data);
        }
        for (auto& node : e.second.mModel->GetNodes()) {
            RenderNode(node, glTFMaterial::AlphaMode::ALPHAMODE_BLEND, data);
        }
    }
}

void VRE::VRE_GameObjRenderSystem::RenderNode(std::shared_ptr<glTFNode> node, glTFMaterial::AlphaMode alphaMode, GameObjectBufferData& data)
{
    if (!node->mMesh)
        return;

    for (auto& prim : node->mMesh->mPrimitives) {

        auto pipelineType = prim->mMaterial.mUnlit ? PipelineType::Unlit : PipelineType::PBR;

        pipelineType = alphaMode == glTFMaterial::AlphaMode::ALPHAMODE_BLEND ?
                                        PipelineType(pipelineType + 2) : prim->mMaterial.mDoubleSided ?
                                            PipelineType(pipelineType + 1) : pipelineType;

        auto& pipeline = mPipelines[pipelineType];

        pipeline->Bind(mSharedContext->mRenderer->GetCurrentCommandBuffer());

        const std::vector<VkDescriptorSet> sets = { mSharedContext->mSceneDescriptorSets[mSharedContext->mRenderer->GetFrameIndex()],
                                                   prim->mMaterial.mDescriptor,
                                                   node->mMesh->mDescriptor,
                                                   mMatBufferDescriptor };

        vkCmdBindDescriptorSets(mSharedContext->mRenderer->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout,
                                0, static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);

        data.mMaterialIndex = prim->mMaterial.mIndex;

        vkCmdPushConstants(mSharedContext->mRenderer->GetCurrentCommandBuffer(), mPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GameObjectBufferData), &data);

        if (prim->mHasIndices)
            vkCmdDrawIndexed(mSharedContext->mRenderer->GetCurrentCommandBuffer(), prim->mIndexCount, 1, prim->mFirstIndex, 0, 0);
        else
            vkCmdDraw(mSharedContext->mRenderer->GetCurrentCommandBuffer(), prim->mVertexCount, 1, 0, 0);
    }

    for (auto& child : node->mChildren)
        RenderNode(child, alphaMode, data);
}

void VRE::VRE_GameObjRenderSystem::CreatePipelineLayouts()
{
    VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t) };
    
    CreateDescSetLayouts();

    std::vector<VkDescriptorSetLayout> descSetLayouts{ mSharedContext->mGlobalDescSetLayout->GetDescriptorSetLayout(),
                                                       mDescSetLayouts.mMaterialDescSetLayout->GetDescriptorSetLayout(),
                                                       mDescSetLayouts.mNodeDescSetLayout->GetDescriptorSetLayout(),
                                                       mDescSetLayouts.mMaterialBufferDescSetLayout->GetDescriptorSetLayout()};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(mSharedContext->mDevice->GetVkDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout!");
}

void VRE::VRE_GameObjRenderSystem::CreatePipelines()
{
    assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

    PipelineConfigInfo pipelineConfig{};
    VRE_Pipeline::GetDefaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.mRenderPass = mSharedContext->mRenderer->GetSwapChainRenderPass();
    pipelineConfig.mPipelineLayout = mPipelineLayout;

    mPipelines.emplace(PipelineType::PBR, std::move(std::make_unique<VRE_Pipeline>(*mSharedContext->mDevice, pipelineConfig, "Shaders/pbr.vert.spv", "Shaders/mat_pbr.frag.spv")));
    mPipelines.emplace(PipelineType::Unlit, std::move(std::make_unique<VRE_Pipeline>(*mSharedContext->mDevice, pipelineConfig, "Shaders/pbr.vert.spv", "Shaders/mat_unlit.frag.spv")));

    for (auto baseType : { PipelineType::PBR, PipelineType::Unlit }) {
        std::string fragShader = baseType == 0 ? "Shaders/mat_pbr.frag.spv" : "Shaders/mat_unlit.frag.spv";

        pipelineConfig.mRasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        mPipelines.emplace(PipelineType(baseType + 1), std::move(std::make_unique<VRE_Pipeline>(*mSharedContext->mDevice, pipelineConfig, "Shaders/pbr.vert.spv", fragShader)));

        pipelineConfig.mColorBlendAttachment.blendEnable = VK_TRUE;
        pipelineConfig.mColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        pipelineConfig.mColorBlendAttachment.dstColorBlendFactor= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        pipelineConfig.mColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        mPipelines.emplace(PipelineType(baseType + 2), std::move(std::make_unique<VRE_Pipeline>(*mSharedContext->mDevice, pipelineConfig, "Shaders/pbr.vert.spv", fragShader)));
    }
}

void VRE::VRE_GameObjRenderSystem::CreateDescSetLayouts()
{
    CreateDescPools();

    mDescSetLayouts.mMaterialDescSetLayout = VRE_DescriptorSetLayout::Builder(*mSharedContext->mDevice)
                                             .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                             .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                             .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                             .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                             .AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                             .Build();

    mDescSetLayouts.mNodeDescSetLayout = VRE_DescriptorSetLayout::Builder(*mSharedContext->mDevice)
                                         .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                                         .Build();

    mDescSetLayouts.mMaterialBufferDescSetLayout = VRE_DescriptorSetLayout::Builder(*mSharedContext->mDevice)
                                                   .AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                                   .Build();
}

void VRE::VRE_GameObjRenderSystem::CreateDescPools()
{
    uint32_t texSampCount = 0;
    uint32_t matCount = 0;
    uint32_t meshCount = 0;

    for (auto& e : *mSharedContext->mGameObjMap) {
        for (auto& objMat : e.second.mModel->GetMaterials()) {
            texSampCount += 5;
            matCount++;
        }
        for (auto& node : e.second.mModel->GetAllNodes())
            if (node->mMesh) meshCount++;
    }

    mDescPool = VRE_DescriptorPool::Builder(*mSharedContext->mDevice)
                    .SetMaxSets((matCount + meshCount) * VRE_SwapChain::MAX_FRAMES_IN_FLIGHT)
                    .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshCount * VRE_SwapChain::MAX_FRAMES_IN_FLIGHT)
                    .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texSampCount * VRE_SwapChain::MAX_FRAMES_IN_FLIGHT)
                    .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1)
                    .Build();
}

void VRE::VRE_GameObjRenderSystem::CreateShaderMatBuffer()
{
    std::vector<ShaderMaterial> shaderMatList;
    for (auto& e : *mSharedContext->mGameObjMap) {
        for (auto& mat : e.second.mModel->GetMaterials()) {
            ShaderMaterial shdMat{};

            shdMat.mBaseColorTextureSet = mat.mBaseColorTexture ? mat.mTexCoordSets.mBaseColor : -1;
            shdMat.mNormalTextureSet = mat.mNormalTexture ? mat.mTexCoordSets.mNormal : -1;
            shdMat.mOcclusionTextureSet = mat.mOcclusionTexture ? mat.mTexCoordSets.mOcclusion : -1;
            shdMat.mEmissiveTextureSet = mat.mEmissiveTexture ? mat.mTexCoordSets.mEmissive : -1;

            shdMat.mEmissiveFactor = mat.mEmissiveFactor;
            shdMat.mEmissiveStrength = mat.mEmissiveStrength;

            shdMat.mAlphaMask = (mat.mAlphaMode == glTFMaterial::AlphaMode::ALPHAMODE_MASK) ? 1.0f : 0.0f;
            shdMat.mAlphaCutoff = mat.mAlphaCutoff;

            if (mat.mPbrWorkflows.mMetallicRoughness) {
                shdMat.mWorkflow = 0.f;
                shdMat.mBaseColorFactor = mat.mBaseColorFactor;
                shdMat.mMetallicFactor = mat.mMetallicFactor;
                shdMat.mRoughnessFactor = mat.mRoughnessFactor;

                shdMat.mPhysicalDescriptorTextureSet = mat.mMetallicRoughnessTexture ? mat.mTexCoordSets.mMetallicRoughness : -1;
            }
            else {
                if (mat.mPbrWorkflows.mSpecularGloss) {
                    shdMat.mWorkflow = 1.0f;

                    shdMat.mPhysicalDescriptorTextureSet = mat.mExtension.mSpecularGlossTexture ? mat.mTexCoordSets.mSpecularGloss : -1;
                    shdMat.mBaseColorTextureSet = mat.mExtension.mDiffuseTexture ? mat.mTexCoordSets.mBaseColor : -1;

                    shdMat.mDiffuseFactor = mat.mExtension.mDiffuseFactor;
                    shdMat.mSpecularFactor = glm::vec4(mat.mExtension.mSpecularFactor, 1.0f);
                }
            }
            shaderMatList.push_back(shdMat);
        }
    }

    VRE_Buffer stagingBuffer(*mSharedContext->mDevice, sizeof(ShaderMaterial), shaderMatList.size() * sizeof(ShaderMaterial),
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBuffer.Map();
    stagingBuffer.WriteToBuffer((void*)shaderMatList.data());

    mShaderMatBuffer = std::make_unique<VRE_Buffer>(*mSharedContext->mDevice, sizeof(ShaderMaterial), shaderMatList.size() * sizeof(ShaderMaterial),
                                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    mSharedContext->mDevice->CopyBuffer(stagingBuffer.GetBuffer(), mShaderMatBuffer->GetBuffer(), sizeof(ShaderMaterial));
}

void VRE::VRE_GameObjRenderSystem::WriteToMaterialDesc()
{
    for (auto& e : *mSharedContext->mGameObjMap) {
        for (auto& objMat : e.second.mModel->GetMaterials()) {
            auto writer = VRE_DescriptorWriter(*mDescSetLayouts.mMaterialDescSetLayout, *mDescPool);
            if (objMat.mPbrWorkflows.mMetallicRoughness) {
                auto info = objMat.mBaseColorTexture ? objMat.mBaseColorTexture->GetDescImageInfo() : mEmptyTexture->GetDescImageInfo();
                writer.WriteImage(0, &info);

                auto info1 = objMat.mMetallicRoughnessTexture ? objMat.mMetallicRoughnessTexture->GetDescImageInfo() : mEmptyTexture->GetDescImageInfo();
                writer.WriteImage(1, &info1);
            }
            else {
                auto info = objMat.mExtension.mDiffuseTexture ? objMat.mExtension.mDiffuseTexture->GetDescImageInfo() : mEmptyTexture->GetDescImageInfo();
                writer.WriteImage(0, &info);

                auto info1 = objMat.mExtension.mSpecularGlossTexture ? objMat.mExtension.mSpecularGlossTexture->GetDescImageInfo() : mEmptyTexture->GetDescImageInfo();
                writer.WriteImage(1,  &info1);
            }
            auto info2 = objMat.mNormalTexture ? objMat.mNormalTexture->GetDescImageInfo() : mEmptyTexture->GetDescImageInfo();
            writer.WriteImage(2, &info2);

            auto info3 = objMat.mOcclusionTexture ? objMat.mOcclusionTexture->GetDescImageInfo() : mEmptyTexture->GetDescImageInfo();
            writer.WriteImage(3, &info3);

            auto info4 = objMat.mEmissiveTexture ? objMat.mEmissiveTexture->GetDescImageInfo() : mEmptyTexture->GetDescImageInfo();
            writer.WriteImage(4, &info4);

            writer.Build(objMat.mDescriptor);
        }
    }
}

void VRE::VRE_GameObjRenderSystem::WriteToNodeDesc()
{
    for (auto& e : *mSharedContext->mGameObjMap)
        for (auto& node : e.second.mModel->GetNodes())
            WriteToNodeDescByNode(node);
}

void VRE::VRE_GameObjRenderSystem::WriteToNodeDescByNode(std::shared_ptr<glTFNode> node)
{
    if (node->mMesh) {
        VkDescriptorSet nodeDescSet;
        auto info = node->mMesh->mBuffer->DescriptorInfo();
        VRE_DescriptorWriter(*mDescSetLayouts.mNodeDescSetLayout, *mDescPool)
                             .WriteBuffer(0, &info)
                             .Build(nodeDescSet);
    }
    for (auto& child : node->mChildren)
        WriteToNodeDescByNode(child);
}

void VRE::VRE_GameObjRenderSystem::WriteToMaterialBufferDesc()
{
    auto info = mShaderMatBuffer->DescriptorInfo();
    VRE_DescriptorWriter(*mDescSetLayouts.mMaterialBufferDescSetLayout, *mDescPool)
                         .WriteBuffer(0, &info)
                         .Build(mMatBufferDescriptor);
}
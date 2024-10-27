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
    CreatePipelineLayouts();
    CreatePipelines();
    CreateShaderMatBuffer();
    WriteToMaterialDesc();
    WriteToNodeDesc();
    WriteToMaterialBufferDesc();
}

void VRE::VRE_GameObjRenderSystem::Render()
{
    //mPipeline->Bind(frameInfo.mCommandBuffer);

    //vkCmdBindDescriptorSets(frameInfo.mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &frameInfo.mDescSet, 0, nullptr);

    //for (auto& e : frameInfo.mGameObjects) {
    //    if (!e.second.mModel)
    //        continue;

    //    auto bufferInfo = e.second.GetBufferInfo(frameInfo.mFrameIndex);

    //    VkDescriptorSet gameObjDescSet;
    //    VRE_DescriptorWriter(*mRenderSystemLayout, frameInfo.mFrameDescPool)
    //        .WriteBuffer(0, &bufferInfo)
    //        .Build(gameObjDescSet);

    //    vkCmdBindDescriptorSets(frameInfo.mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 1, 1, &gameObjDescSet, 0, nullptr);

    //    e.second.mModel->Bind(frameInfo.mCommandBuffer);
    //    e.second.mModel->Draw(frameInfo.mCommandBuffer);
    //}
}

void VRE::VRE_GameObjRenderSystem::CreatePipelineLayouts()
{
    VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GameObjectBufferData) };
    
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
    mPipelines.emplace(MaterialType::PBR, std::move(std::make_unique<VRE_Pipeline>(*mSharedContext->mDevice, pipelineConfig, "Shaders/pbr.vert.spv", "Shaders/mat_pbr.frag.spv")));
    mPipelines.emplace(MaterialType::Unlit, std::move(std::make_unique<VRE_Pipeline>(*mSharedContext->mDevice, pipelineConfig, "Shaders/pbr.vert.spv", "Shaders/mat_unlit.frag.spv")));
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

    for (auto& e : mSharedContext->mGameObjMap) {
        for (auto& objMat : e.second.GetModel()->GetMaterials()) {
            texSampCount += 5;
            matCount++;
        }
        for (auto& node : e.second.GetModel()->GetAllNodes())
            if (node->mMesh) meshCount++;
    }

    mDescPools.resize(VRE_SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < VRE_SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        mDescPools[i] = VRE_DescriptorPool::Builder(*mSharedContext->mDevice)
                        .SetMaxSets(matCount + meshCount)
                        .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshCount)
                        .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texSampCount)
                        .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1)
                        .Build();
    }
}

void VRE::VRE_GameObjRenderSystem::CreateShaderMatBuffer()
{
    std::vector<ShaderMaterial> shaderMatList;
    for (auto& e : mSharedContext->mGameObjMap) {
        for (auto& mat : e.second.GetModel()->GetMaterials()) {
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
    stagingBuffer.WriteToBuffer(&shaderMatList);

    mShaderMatBuffer = std::make_unique<VRE_Buffer>(*mSharedContext->mDevice, sizeof(ShaderMaterial), shaderMatList.size() * sizeof(ShaderMaterial),
                                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    mSharedContext->mDevice->CopyBuffer(stagingBuffer.GetBuffer(), mShaderMatBuffer->GetBuffer(), sizeof(ShaderMaterial));
}

void VRE::VRE_GameObjRenderSystem::WriteToMaterialDesc()
{
    for (auto& e : mSharedContext->mGameObjMap) {
        for (auto& objMat : e.second.GetModel()->GetMaterials()) {
            VkDescriptorSet materialDescSet;
            auto writer = VRE_DescriptorWriter(*mDescSetLayouts.mMaterialDescSetLayout, *mDescPools[mSharedContext->mRenderer->GetFrameIndex()]);
            if (objMat.mPbrWorkflows.mMetallicRoughness) {
                writer.WriteImage(0, objMat.mBaseColorTexture ? &objMat.mBaseColorTexture->GetDescImageInfo() : &VkDescriptorImageInfo());
                writer.WriteImage(1, objMat.mMetallicRoughnessTexture ? &objMat.mMetallicRoughnessTexture->GetDescImageInfo() : &VkDescriptorImageInfo());
            }
            else {
                writer.WriteImage(0, objMat.mExtension.mDiffuseTexture ? &objMat.mExtension.mDiffuseTexture->GetDescImageInfo() : &VkDescriptorImageInfo());
                writer.WriteImage(1, objMat.mExtension.mSpecularGlossTexture ? &objMat.mExtension.mSpecularGlossTexture->GetDescImageInfo() : &VkDescriptorImageInfo());
            }
            writer.WriteImage(2, objMat.mNormalTexture ? &objMat.mNormalTexture->GetDescImageInfo() : &VkDescriptorImageInfo());
            writer.WriteImage(3, objMat.mOcclusionTexture ? &objMat.mOcclusionTexture->GetDescImageInfo() : &VkDescriptorImageInfo());
            writer.WriteImage(4, objMat.mEmissiveTexture ? &objMat.mEmissiveTexture->GetDescImageInfo() : &VkDescriptorImageInfo());
            writer.Build(materialDescSet);
            vkCmdBindDescriptorSets(mSharedContext->mRenderer->GetCurrentCommandBuffer(),
                                    VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 1, 1, &materialDescSet, 0, nullptr);
        }
    }
}

void VRE::VRE_GameObjRenderSystem::WriteToNodeDesc()
{
    for (auto& e : mSharedContext->mGameObjMap)
        for (auto& node : e.second.GetModel()->GetNodes())
            WriteToNodeDescByNode(node);
}

void VRE::VRE_GameObjRenderSystem::WriteToNodeDescByNode(std::shared_ptr<glTFNode> node)
{
    if (node->mMesh) {
        VkDescriptorSet nodeDescSet;
        VRE_DescriptorWriter(*mDescSetLayouts.mNodeDescSetLayout, *mDescPools[mSharedContext->mRenderer->GetFrameIndex()])
            .WriteBuffer(0, &node->mMesh->mBuffer->DescriptorInfo())
            .Build(nodeDescSet);
        vkCmdBindDescriptorSets(mSharedContext->mRenderer->GetCurrentCommandBuffer(),
            VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 2, 1, &nodeDescSet, 0, nullptr);
    }
    for (auto& child : node->mChildren)
        WriteToNodeDescByNode(child);
}

void VRE::VRE_GameObjRenderSystem::WriteToMaterialBufferDesc()
{
    VkDescriptorSet shdMatDescSet;
    VRE_DescriptorWriter(*mDescSetLayouts.mMaterialBufferDescSetLayout, *mDescPools[mSharedContext->mRenderer->GetFrameIndex()])
        .WriteBuffer(0, &mShaderMatBuffer->DescriptorInfo())
        .Build(shdMatDescSet);
    vkCmdBindDescriptorSets(mSharedContext->mRenderer->GetCurrentCommandBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 3, 1, &shdMatDescSet, 0, nullptr);
}
#include "VRE_LightRenderSystem.h"
#include <stdexcept>
#include <cassert>
#include <iostream>
#include "gtc/matrix_transform.hpp"

namespace VRE {
    struct PointLightPC {
        glm::vec4 mPosition{};
        glm::vec4 mColor{};
        float mRadius = 0.f;
    };
}

VRE::VRE_LightRenderSystem::VRE_LightRenderSystem(VRE_SharedContext* sceneContext)
    : mSceneContext(sceneContext) {}

VRE::VRE_LightRenderSystem::~VRE_LightRenderSystem()
{
    vkDestroyPipelineLayout(mSceneContext->mDevice->GetVkDevice(), mPipelineLayout, nullptr);
}

void VRE::VRE_LightRenderSystem::Init()
{
    CreatePipelineLayout(mSceneContext->mGlobalDescSetLayout->GetDescriptorSetLayout());
    CreatePipeline(mSceneContext->mRenderer->GetSwapChainRenderPass());
}

void VRE::VRE_LightRenderSystem::Update(UBO &ubo, float dt)
{
    int index = 0;
    auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * dt, { 0.f, -1.f, 0.f });

    for (auto& light : mSceneContext->mPointLights) {
        assert(index < MAX_LIGHTS && "Point lights exceed maximum number specified in SharedContext.h!");

        light.mPosition = rotateLight * light.mPosition;

        ubo.mPointLights[index].mPosition = light.mPosition;
        ubo.mPointLights[index].mColor = glm::vec4(light.mColor, light.mLightIntensity);
        ++index;
    }

    ubo.mActiveLightCount = index;
}

void VRE::VRE_LightRenderSystem::Render()
{
    mPipeline->Bind(mSceneContext->mRenderer->GetCurrentCommandBuffer());
    VkDescriptorSet* descSet = &mSceneContext->mSceneDescriptorSets[mSceneContext->mRenderer->GetFrameIndex()];
    vkCmdBindDescriptorSets(mSceneContext->mRenderer->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, descSet, 0, nullptr);

    for (auto& light : mSceneContext->mPointLights) {
        PointLightPC pc;
        pc.mPosition = light.mPosition;
        pc.mColor = glm::vec4(light.mColor, light.mLightIntensity);
        pc.mRadius = light.mScale;

        vkCmdPushConstants(mSceneContext->mRenderer->GetCurrentCommandBuffer(),
                           mPipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                           0,
                           sizeof(PointLightPC),
                           &pc);

        vkCmdDraw(mSceneContext->mRenderer->GetCurrentCommandBuffer(), 6, 1, 0, 0);
    }
}

void VRE::VRE_LightRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout descSetLayout)
{
    VkPushConstantRange pushConstantRange{VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPC) };

    std::vector<VkDescriptorSetLayout> descSetLayouts{ descSetLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(mSceneContext->mDevice->GetVkDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout!");
}

void VRE::VRE_LightRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
    assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

    PipelineConfigInfo pipelineConfig{};
    VRE_Pipeline::GetDefaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.mAttributeDescriptions.clear();
    pipelineConfig.mBindingDescriptions.clear();
    pipelineConfig.mRenderPass = renderPass;
    pipelineConfig.mPipelineLayout = mPipelineLayout;
    mPipeline = std::make_unique<VRE_Pipeline>(*mSceneContext->mDevice, pipelineConfig, "Shaders/point_light.vert.spv", "Shaders/point_light.frag.spv");
}

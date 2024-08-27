#include "VRE_App.h"
#include <iostream>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

namespace VRE {
    //push constant data.
    struct SimplePCData {
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };
}

VRE::VRE_App::VRE_App()
{
    LoadModels();
    CreatePipelineLayout();
    RecreateSwapChain();
    CreateCommandBuffers();
}

VRE::VRE_App::~VRE_App()
{
    vkDestroyPipelineLayout(mDevice.device(), mPipelineLayout, nullptr);
}

void VRE::VRE_App::Run()
{
    while (!mWindow.ShouldClose()) {
        glfwPollEvents();
        DrawFrame();
    }
    vkDeviceWaitIdle(mDevice.device());
}

void VRE::VRE_App::LoadModels()
{
    std::vector<VRE_Model::Vertex> vertices{
     {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
     {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
     {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}} };
    mModel = std::make_unique<VRE_Model>(mDevice, vertices);
}

void VRE::VRE_App::CreatePipelineLayout()
{
    VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                           0,
                                           sizeof(SimplePCData) };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(mDevice.device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
        std::cout << "Failed to create pipeline layout!" << std::endl;
}

void VRE::VRE_App::CreatePipeline()
{
    assert(mSwapChain != nullptr && "Cannot create pipeline before swap chain");
    assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    VRE_Pipeline::GetDefaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = mSwapChain->getRenderPass();
    pipelineConfig.pipelineLayout = mPipelineLayout;
    mPipeline = std::make_unique<VRE_Pipeline>(
        mDevice,
        pipelineConfig,
        "Shaders/test_shader.vert.spv",
        "Shaders/test_shader.frag.spv");
}

void VRE::VRE_App::CreateCommandBuffers()
{
    mCommandBuffers.resize(mSwapChain->imageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = mDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

    if (vkAllocateCommandBuffers(mDevice.device(), &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");
    
}

void VRE::VRE_App::DrawFrame()
{
    uint32_t imageIndex;
    auto result = mSwapChain->acquireNextImage(&imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swap chain image!");

    RecordCommandBuffer(imageIndex);
    result = mSwapChain->submitCommandBuffers(&mCommandBuffers[imageIndex], &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        mWindow.HasWindowResized()) {
        mWindow.ResetWindowResizedFlag();
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS)
        throw std::runtime_error("failed to present swap chain image!");
}

void VRE::VRE_App::RecreateSwapChain()
{
    auto extent = mWindow.GetExtent();

    while (extent.width == 0 || extent.height == 0) {
        extent = mWindow.GetExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mDevice.device());

    if (mSwapChain == nullptr)
        mSwapChain = std::make_unique<VRE_SwapChain>(mDevice, extent);
    else {
        mSwapChain = std::make_unique<VRE_SwapChain>(mDevice, extent, std::move(mSwapChain));
        if (mSwapChain->imageCount() != mCommandBuffers.size()) {
            FreeCommandBuffers();
            CreateCommandBuffers();
        }
    }

    CreatePipeline();
}

void VRE::VRE_App::FreeCommandBuffers() {
    vkFreeCommandBuffers(
        mDevice.device(),
        mDevice.getCommandPool(),
        static_cast<uint32_t>(mCommandBuffers.size()),
        mCommandBuffers.data());
    mCommandBuffers.clear();
}

void VRE::VRE_App::RecordCommandBuffer(int imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(mCommandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mSwapChain->getRenderPass();
    renderPassInfo.framebuffer = mSwapChain->getFrameBuffer(imageIndex);

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = mSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(mCommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(mSwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(mSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{ {0, 0}, mSwapChain->getSwapChainExtent() };
    vkCmdSetViewport(mCommandBuffers[imageIndex], 0, 1, &viewport);
    vkCmdSetScissor(mCommandBuffers[imageIndex], 0, 1, &scissor);

    mPipeline->Bind(mCommandBuffers[imageIndex]);
    mModel->Bind(mCommandBuffers[imageIndex]);

    for (int i = 0; i < 4; i++) {
        SimplePCData data{};
        data.offset = { 0.0f, -0.4f + i * 0.25f };
        data.color = { 0.0f, 0.0f, 0.2f + 0.2f * i };

        vkCmdPushConstants(mCommandBuffers[imageIndex],
                           mPipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                           0,
                           sizeof(SimplePCData),
                           &data);

        mModel->Draw(mCommandBuffers[imageIndex]);
    }

    vkCmdEndRenderPass(mCommandBuffers[imageIndex]);
    if (vkEndCommandBuffer(mCommandBuffers[imageIndex]) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

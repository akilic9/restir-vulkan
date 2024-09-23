#include "VRE_Renderer.h"
#include <stdexcept>
#include <array>

VRE::VRE_Renderer::VRE_Renderer(VRE_Window& window, VRE_Device& device)
    : mWindow(window)
    , mDevice(device)
    , mCurrentImageIndex(0)
    , mDrawStarted(false)
{
    RecreateSwapChain();
    CreateCommandBuffers();
}

VRE::VRE_Renderer::~VRE_Renderer()
{
    FreeCommandBuffers();
}

VkCommandBuffer VRE::VRE_Renderer::BeginDraw()
{
    assert(!mDrawStarted && "Can't call BeginDraw while already in progress.");

    auto result = mSwapChain->AcquireNextImage(&mCurrentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
        return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to acquire swap chain image!");

    mDrawStarted = true;
    auto commandBuffer = GetCurrentCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording command buffer!");

    return commandBuffer;
}

void VRE::VRE_Renderer::EndDraw()
{
    assert(mDrawStarted && "Can't call EndDraw while frame is not in progress.");
    auto commandBuffer = GetCurrentCommandBuffer();

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer!");

    auto result = mSwapChain->SubmitCommandBuffers(&commandBuffer, &mCurrentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        mWindow.HasWindowResized()) {
        mWindow.ResetWindowResizedFlag();
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to present swap chain image!");

    mDrawStarted = false;
    mCurrentFrameIndex = (mCurrentFrameIndex + 1) % VRE_SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void VRE::VRE_Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(mDrawStarted && "Can't call BeginSwapChainRenderPass if frame is not in progress.");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame.");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mSwapChain->GetRenderPass();
    renderPassInfo.framebuffer = mSwapChain->GetFrameBuffer(mCurrentImageIndex);

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = mSwapChain->GetSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(mSwapChain->GetSwapChainExtent().width);
    viewport.height = static_cast<float>(mSwapChain->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{ {0, 0}, mSwapChain->GetSwapChainExtent() };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VRE::VRE_Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(mDrawStarted && "Can't call EndSwapChainRenderPass if frame is not in progress.");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame.");

    vkCmdEndRenderPass(commandBuffer);
}

void VRE::VRE_Renderer::CreateCommandBuffers()
{
    mCommandBuffers.resize(VRE_SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = mDevice.GetCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

    if (vkAllocateCommandBuffers(mDevice.GetVkDevice(), &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers!");
}

void VRE::VRE_Renderer::RecreateSwapChain()
{
    auto extent = mWindow.GetExtent();

    while (extent.width == 0 || extent.height == 0) { //Pause if the window is minimized as it can cause swapchain to become out of date.
        extent = mWindow.GetExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mDevice.GetVkDevice());

    if (mSwapChain == nullptr)
        mSwapChain = std::make_unique<VRE_SwapChain>(mDevice, extent);
    else {
        std::shared_ptr<VRE_SwapChain> oldChain = std::move(mSwapChain);
        mSwapChain = std::make_unique<VRE_SwapChain>(mDevice, extent, oldChain);

        if (!oldChain->CompareSwapFormats(*mSwapChain.get()))
            throw std::runtime_error("Swap chain image (or depth) format has changed!");
    }
}

void VRE::VRE_Renderer::FreeCommandBuffers()
{
    vkFreeCommandBuffers(mDevice.GetVkDevice(), mDevice.GetCommandPool(), static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());
    mCommandBuffers.clear();
}
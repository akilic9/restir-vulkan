#include "VRE_SwapChain.h"
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace VRE {
    
    VRE_SwapChain::VRE_SwapChain(VRE_Device &deviceRef, VkExtent2D extent)
        : mDevice{deviceRef}
        , mWindowExtent{extent}
    {
        Init();
    }
    
    VRE_SwapChain::VRE_SwapChain(VRE_Device& deviceRef, VkExtent2D extent, std::shared_ptr<VRE_SwapChain> previous)
        : mDevice{deviceRef}
        , mWindowExtent{extent}
        , mOldSwapChain{previous} 
    {
        Init();
        mOldSwapChain = nullptr;
    }
    
    VRE_SwapChain::~VRE_SwapChain()
    {
        for (auto imageView : mSwapChainImageViews)
            vkDestroyImageView(mDevice.GetVkDevice(), imageView, nullptr);
    
        mSwapChainImageViews.clear();
    
        if (mSwapChain != nullptr) {
            vkDestroySwapchainKHR(mDevice.GetVkDevice(), mSwapChain, nullptr);
            mSwapChain = nullptr;
        }
    
        for (int i = 0; i < mDepthImages.size(); i++) {
            vkDestroyImageView(mDevice.GetVkDevice(), mDepthImageViews[i], nullptr);
            vkDestroyImage(mDevice.GetVkDevice(), mDepthImages[i], nullptr);
            vkFreeMemory(mDevice.GetVkDevice(), mDepthImageMemorys[i], nullptr);
        }
    
        for (auto framebuffer : mSwapChainFramebuffers)
            vkDestroyFramebuffer(mDevice.GetVkDevice(), framebuffer, nullptr);
    
        vkDestroyRenderPass(mDevice.GetVkDevice(), mRenderPass, nullptr);
    
        // cleanup synchronization objects
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(mDevice.GetVkDevice(), mRenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(mDevice.GetVkDevice(), mImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(mDevice.GetVkDevice(), mInFlightFences[i], nullptr);
        }
    }
    
    void VRE_SwapChain::Init()
    {
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateDepthResources();
        CreateFramebuffers();
        CreateSyncObjects();
    }
    
    VkResult VRE_SwapChain::AcquireNextImage(uint32_t *imageIndex)
    {
        vkWaitForFences(mDevice.GetVkDevice(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    
        VkResult result = vkAcquireNextImageKHR(mDevice.GetVkDevice(), mSwapChain, std::numeric_limits<uint64_t>::max(),
                                                mImageAvailableSemaphores[mCurrentFrame],  // must be a not signaled semaphore
                                                VK_NULL_HANDLE, imageIndex);
    
        return result;
    }
    
    VkResult VRE_SwapChain::SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex)
    {
        if (mImagesInFlight[*imageIndex] != VK_NULL_HANDLE)
            vkWaitForFences(mDevice.GetVkDevice(), 1, &mImagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
    
        mImagesInFlight[*imageIndex] = mInFlightFences[mCurrentFrame];
    
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
        VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[mCurrentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
    
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;
    
        VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mCurrentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
    
        vkResetFences(mDevice.GetVkDevice(), 1, &mInFlightFences[mCurrentFrame]);
    
        if (vkQueueSubmit(mDevice.GraphicsQueue(), 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit draw command buffer!");
    
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
    
        VkSwapchainKHR swapChains[] = {mSwapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
    
        presentInfo.pImageIndices = imageIndex;
    
        auto result = vkQueuePresentKHR(mDevice.PresentQueue(), &presentInfo);
    
        mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    
        return result;
    }
    
    void VRE_SwapChain::CreateSwapChain() {
        SwapChainSupportDetails swapChainSupport = mDevice.GetSwapChainSupport();
    
        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.mFormats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.mPresentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.mCapabilities);
    
        uint32_t imageCount = swapChainSupport.mCapabilities.minImageCount + 1;
    
        if (swapChainSupport.mCapabilities.maxImageCount > 0
            && imageCount > swapChainSupport.mCapabilities.maxImageCount) {
            imageCount = swapChainSupport.mCapabilities.maxImageCount;
        }
    
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = mDevice.Surface();
    
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
        QueueFamilyIndices indices = mDevice.FindPhysicalQueueFamilies();
        uint32_t queueFamilyIndices[] = {indices.mGraphicsFamily, indices.mPresentFamily};
    
        if (indices.mGraphicsFamily != indices.mPresentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;      // Optional
            createInfo.pQueueFamilyIndices = nullptr;  // Optional
        }
    
        createInfo.preTransform = swapChainSupport.mCapabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
    
        createInfo.oldSwapchain = mOldSwapChain == nullptr ? VK_NULL_HANDLE : mOldSwapChain->mSwapChain;
    
        if (vkCreateSwapchainKHR(mDevice.GetVkDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
          throw std::runtime_error("Failed to create swap chain!");
    
        // We only specified a minimum number of images in the swap chain, so the implementation is
        // allowed to create a swap chain with more. That's why we'll first query the final number of
        // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
        // retrieve the handles.
        vkGetSwapchainImagesKHR(mDevice.GetVkDevice(), mSwapChain, &imageCount, nullptr);
        mSwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(mDevice.GetVkDevice(), mSwapChain, &imageCount, mSwapChainImages.data());
    
        mSwapChainImageFormat = surfaceFormat.format;
        mSwapChainExtent = extent;
    }
    
    void VRE_SwapChain::CreateImageViews()
    {
        mSwapChainImageViews.resize(mSwapChainImages.size());
    
        for (size_t i = 0; i < mSwapChainImages.size(); i++) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = mSwapChainImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = mSwapChainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
    
            if (vkCreateImageView(mDevice.GetVkDevice(), &viewInfo, nullptr, &mSwapChainImageViews[i]) != VK_SUCCESS)
              throw std::runtime_error("Failed to create texture image view!");
        }
    }
    
    void VRE_SwapChain::CreateRenderPass()
    {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = FindDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = GetSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
    
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstSubpass = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;
    
        if (vkCreateRenderPass(mDevice.GetVkDevice(), &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
            throw std::runtime_error("Failed to create render pass!");
    }
    
    void VRE_SwapChain::CreateFramebuffers()
    {
        mSwapChainFramebuffers.resize(GetImageCount());
        for (size_t i = 0; i < GetImageCount(); i++) {
            std::array<VkImageView, 2> attachments = {mSwapChainImageViews[i], mDepthImageViews[i]};
    
            VkExtent2D swapChainExtent = GetSwapChainExtent();
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = mRenderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;
    
            if (vkCreateFramebuffer(mDevice.GetVkDevice(), &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create framebuffer!");
        }
    }
    
    void VRE_SwapChain::CreateDepthResources()
    {
        VkFormat depthFormat = FindDepthFormat();
        mSwapChainDepthFormat = depthFormat;
        VkExtent2D swapChainExtent = GetSwapChainExtent();
    
        mDepthImages.resize(GetImageCount());
        mDepthImageMemorys.resize(GetImageCount());
        mDepthImageViews.resize(GetImageCount());
    
        for (int i = 0; i < mDepthImages.size(); i++) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = swapChainExtent.width;
            imageInfo.extent.height = swapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;
    
            mDevice.CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImages[i], mDepthImageMemorys[i]);
    
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = mDepthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
    
            if (vkCreateImageView(mDevice.GetVkDevice(), &viewInfo, nullptr, &mDepthImageViews[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create texture image view!");
        }
    }
    
    void VRE_SwapChain::CreateSyncObjects()
    {
        mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        mImagesInFlight.resize(GetImageCount(), VK_NULL_HANDLE);
    
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(mDevice.GetVkDevice(), &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS
                || vkCreateSemaphore(mDevice.GetVkDevice(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS
                || vkCreateFence(mDevice.GetVkDevice(), &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create synchronization objects for a frame!");
            }
        }
    }
    
    VkSurfaceFormatKHR VRE_SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        for (const auto &availableFormat : availableFormats)
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
    
        return availableFormats[0];
    }
    
    VkPresentModeKHR VRE_SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        for (const auto &availablePresentMode : availablePresentModes)
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                std::cout << "Present mode: Mailbox" << std::endl;
                return availablePresentMode;
            }
    
        for (const auto &availablePresentMode : availablePresentModes)
            if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                std::cout << "Present mode: Immediate" << std::endl;
                return availablePresentMode;
            }
    
        std::cout << "Present mode: V-Sync" << std::endl;
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    
    VkExtent2D VRE_SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;
        else {
            VkExtent2D actualExtent = mWindowExtent;
            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
    
            return actualExtent;
        }
    }
    
    VkFormat VRE_SwapChain::FindDepthFormat()
    {
        return mDevice.FindSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                           VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}

#pragma once
#include "VRE_Window.h"
#include "VRE_Device.h"
#include "VRE_SwapChain.h"

#include <memory>
#include <vector>
#include <cassert>

namespace VRE {
    class VRE_Renderer
    {
    public:
        VRE_Renderer(VRE_Window& window, VRE_Device& device);
        ~VRE_Renderer();

        VRE_Renderer(const VRE_Renderer&) = delete;
        VRE_Renderer& operator=(const VRE_Renderer&) = delete;

        VkCommandBuffer BeginDraw();
        void EndDraw();
        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

        bool IsDrawInProgress() const { return mDrawStarted; }

        VkRenderPass GetSwapChainRenderPass() const { return mSwapChain->GetRenderPass(); }

        float GetAspectRatio() const { return mSwapChain->GetExtentAspectRatio(); };

        VkCommandBuffer GetCurrentCommandBuffer() const {
            assert(mDrawStarted && "Cannot get command buffer when frame not in progress.");
            return mCommandBuffers[mCurrentFrameIndex];
        }

        int GetFrameIndex() const {
            assert(mDrawStarted && "Cannot get frame index when frame not in progress.");
            return mCurrentFrameIndex;
        }
    private:
        void CreateCommandBuffers();
        void RecreateSwapChain();
        void FreeCommandBuffers();

        VRE_Window& mWindow;
        VRE_Device& mDevice;
        std::unique_ptr<VRE_SwapChain> mSwapChain;
        std::vector<VkCommandBuffer> mCommandBuffers;

        uint32_t mCurrentImageIndex;
        int mCurrentFrameIndex;
        bool mDrawStarted;
    };
}
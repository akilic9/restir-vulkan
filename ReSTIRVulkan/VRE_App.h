#pragma once
#include <string>
#include "VRE_Window.h"
#include "VRE_Pipeline.h"
#include "VRE_Device.h"
#include "VRE_SwapChain.h"
#include "VRE_Model.h"

#include <memory>
#include <vector>

namespace VRE {
    class VRE_App
    {
    public:
        static const int DEF_WINDOW_WIDTH = 1600;
        static const int DEF_WINDOW_HEIGHT = 900;
        inline static const std::string DEF_WINDOW_TITLE = "ReSTIR Vulkan";

        VRE_App();
        ~VRE_App();

        VRE_App(const VRE_App&) = delete;
        VRE_App& operator=(const VRE_App&) = delete;

        void Run();

    private:
        void LoadModels();
        void CreatePipelineLayout();
        void CreatePipeline();
        void CreateCommandBuffers();
        void DrawFrame();
        void RecreateSwapChain();
        void RecordCommandBuffer(int imageIndex);
        void FreeCommandBuffers();

        VRE_Window mWindow{ DEF_WINDOW_WIDTH, DEF_WINDOW_HEIGHT, DEF_WINDOW_TITLE };
        VRE_Device mDevice{ mWindow };
        std::unique_ptr<VRE_SwapChain> mSwapChain;
        std::unique_ptr<VRE_Pipeline> mPipeline;
        VkPipelineLayout mPipelineLayout;
        std::vector<VkCommandBuffer> mCommandBuffers;
        std::unique_ptr<VRE_Model> mModel;
    };
}


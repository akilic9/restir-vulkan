/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*   Overvoorde, A. (2017). Khronos Vulkan Tutorial. [online] Vulkan.org. Available at: https://docs.vulkan.org/tutorial/latest/00_Introduction.html (Accessed 07 June 2024).
*/
#pragma once
#include "VRE_Device.h"
#include "VRE_SharedContext.h"
#include "VRE_Pipeline.h"

namespace VRE {
    class VRE_LightRenderSystem
    {
    public:
        VRE_LightRenderSystem(VRE_SharedContext* sceneContext);
        ~VRE_LightRenderSystem();

        VRE_LightRenderSystem(const VRE_LightRenderSystem&) = delete;
        VRE_LightRenderSystem& operator=(const VRE_LightRenderSystem&) = delete;

        void Init();
        void Update(UBO &ubo, float dt);
        void Render();

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout descSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

        std::unique_ptr<VRE_Pipeline> mPipeline;
        VkPipelineLayout mPipelineLayout;
        VRE_SharedContext* mSceneContext;
    };
}
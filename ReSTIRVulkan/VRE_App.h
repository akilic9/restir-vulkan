/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*   Overvoorde, A. (2017). Khronos Vulkan Tutorial. [online] Vulkan.org. Available at: https://docs.vulkan.org/tutorial/latest/00_Introduction.html (Accessed 07 June 2024).
*   lukasino 1214 (2022). Textures - Vulkan / Game Engine tutorial[0]. [online] YouTube. Available at: https://www.youtube.com/watch?v=_AitmLEnP28 and https://github.com/lukasino1214/Game-Engine-Tutorial (Accessed 10 Aug. 2024).
*/

#pragma once
#include <string>
#include "VRE_Window.h"
#include "VRE_Device.h"
#include "VRE_Renderer.h"
#include "VRE_Descriptor.h"
#include "VRE_PointLight.h"
#include "VRE_LightRenderSystem.h"
#include "VRE_Camera.h"
#include "VRE_InputListener.h"
#include "VRE_GameObjectManager.h"
#include "VRE_GameObjRenderSystem.h"

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

        void Init();
        void Update(float dt, UBO& ubo);
        void Render(UBO& ubo);

    private:
        void LoadObjects();

        VRE_Window mWindow{ DEF_WINDOW_WIDTH, DEF_WINDOW_HEIGHT, DEF_WINDOW_TITLE };
        VRE_Device mDevice{ mWindow };
        VRE_Renderer mRenderer;
        VRE_SharedContext mSceneContext;
        std::unique_ptr<VRE_DescriptorPool> mDescriptorPool;
        std::vector<std::unique_ptr<VRE_DescriptorPool>> mObjectPools;
        std::vector<std::unique_ptr<VRE_Buffer>> mSceneUBOs;
        VRE_LightRenderSystem mPLRenderSystem;
        VRE_GameObjectManager mGameObjectManager;
        VRE_GameObjRenderSystem mGameObjRenderSystem;
        VRE_InputListener mInputListener;
        VRE_Camera mCamera;
    };
}
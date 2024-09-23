#pragma once
#include <string>
#include "VRE_Window.h"
#include "VRE_Device.h"
#include "VRE_Model.h"
#include "VRE_Renderer.h"
#include "VRE_GameObject.h"
#include "VRE_Descriptor.h"
#include "VRE_PointLight.h"
#include "VRE_Texture.h"

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
        void LoadObjects();

        VRE_Window mWindow{ DEF_WINDOW_WIDTH, DEF_WINDOW_HEIGHT, DEF_WINDOW_TITLE };
        VRE_Device mDevice{ mWindow };
        VRE_Renderer mRenderer;
        std::unique_ptr<VRE_DescriptorPool> mDescriptorPool;
        VRE_GameObject::GameObjectsMap mGameObjects;
        std::vector<VRE_PointLight> mPointLights;
        std::shared_ptr<VRE_Texture> mTexture;
    };
}


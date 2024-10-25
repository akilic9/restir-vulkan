/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*   Overvoorde, A. (2017). Khronos Vulkan Tutorial. [online] Vulkan.org. Available at: https://docs.vulkan.org/tutorial/latest/00_Introduction.html (Accessed 07 June 2024).
*   Wikipedia Contributors (2020). Blinn–Phong reflection model. [online] Wikipedia. Available at: https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model (Accessed 15 Aug. 2024).
*   lukasino 1214 (2022). Textures - Vulkan / Game Engine tutorial[0]. [online] YouTube. Available at: https://www.youtube.com/watch?v=_AitmLEnP28 and https://github.com/lukasino1214/Game-Engine-Tutorial (Accessed 10 Aug. 2024).
*   Blanco, V. (2020). Vulkan Guide. [online] Vulkan Guide. Available at: https://vkguide.dev/ (Accessed 07 July 2024).
*   GitHub. (n.d.). tinyobjloader/tinyobjloader. [online] Available at: https://github.com/tinyobjloader/tinyobjloader (Accessed 10 Jul. 2024).
*   GitHub. (2020). g-truc/glm. [online] Available at: https://github.com/g-truc/glm (Accessed 10 Jun. 2024).
*   GitHub. (2021). nothings/stb. [online] Available at: https://github.com/nothings/stb (Accessed 1 Aug. 2024).
*   GLFW. (n.d.). An OpenGL library. [online] Available at: https://www.glfw.org/ (Accessed 7 Jun. 2024).
*/

#include <imgui.h>

#include "VRE_App.h"
#include "VRE_Buffer.h"

#include <iostream>
#include <array>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/constants.hpp>

VRE::VRE_App::VRE_App()
    : mRenderer(mWindow, mDevice)
    , mPLRenderSystem(&mSceneContext)
    , mGameObjectManager(&mSceneContext)
    , mGameObjRenderSystem(&mSceneContext)
{
    Init();
}

VRE::VRE_App::~VRE_App() {}

// TODO: This function can be simplified.
void VRE::VRE_App::Run()
{
    auto startTime = std::chrono::high_resolution_clock::now();

    while (!mWindow.ShouldClose()) {
        glfwPollEvents();

        const auto currentTime = std::chrono::high_resolution_clock::now();
        const float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        std::cout << 1.0f / deltaTime << std::endl;
        startTime = currentTime;

        float aspRatio = mRenderer.GetAspectRatio();
        mCamera.SetPerspectiveProjection(glm::radians(50.f), aspRatio, 0.1f, 1000.f);

        // TODO: Can probably optimize this using GLFW callback function.
        mInputListener.Move(mWindow.GetGLFWwindow(), deltaTime, mCamera);

        if (auto commandBuffer = mRenderer.BeginDraw()) {
            const int frameIndex = mRenderer.GetFrameIndex();
            UBO ubo;

            Update(deltaTime, ubo);
            Render(ubo);

            mRenderer.EndDraw();
        }
    }
    vkDeviceWaitIdle(mDevice.GetVkDevice());
}

void VRE::VRE_App::Init()
{
    //Create descriptor pool for global data.
    mDescriptorPool = VRE_DescriptorPool::Builder(mDevice)
                      .SetMaxSets(VRE_SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VRE_SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .Build();

    for (int i = 0; i < VRE_SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        mSceneUBOs.push_back(std::make_unique<VRE_Buffer>(mDevice, sizeof(UBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
        mSceneUBOs[i]->Map();
    }

    mSceneContext.mGlobalDescSetLayout = VRE_DescriptorSetLayout::Builder(mDevice)
                                         .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                         .Build();


    for (int i = 0; i < VRE_SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        auto bufferInfo = mSceneUBOs[i]->DescriptorInfo();
        mSceneContext.mSceneDescriptorSets.push_back(VkDescriptorSet());
        VRE_DescriptorWriter(*mSceneContext.mGlobalDescSetLayout, *mDescriptorPool)
                             .WriteBuffer(0, &bufferInfo)
                             .Build(mSceneContext.mSceneDescriptorSets[i]);
    }

    mPLRenderSystem.Init();
    mGameObjectManager.Init();
    mGameObjRenderSystem.Init();
    mCamera = VRE_Camera();
    mInputListener = VRE_InputListener();
    LoadObjects();
}

void VRE::VRE_App::Update(float dt, UBO& ubo)
{
    ubo.mProjectionMat = mCamera.GetProjection();
    ubo.mViewMat = mCamera.GetViewMat();
    ubo.mInvViewMat = mCamera.GetInvViewMat();
    mPLRenderSystem.Update(ubo, dt);

    mSceneUBOs[mRenderer.GetFrameIndex()]->WriteToBuffer(&ubo);
    mSceneUBOs[mRenderer.GetFrameIndex()]->Flush();

    mGameObjectManager.Update(dt);
}

void VRE::VRE_App::Render(UBO& ubo)
{
    mRenderer.BeginSwapChainRenderPass(mRenderer.GetCurrentCommandBuffer());
    mPLRenderSystem.RenderLights();
    mGameObjRenderSystem.RenderGameObjects();
    mRenderer.EndSwapChainRenderPass(mRenderer.GetCurrentCommandBuffer());
}

void VRE::VRE_App::LoadObjects()
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //std::shared_ptr<VRE_glTFModel> model = std::make_shared<VRE_glTFModel>(mDevice, "Resources/Models/Duck/", "Duck");
    //model->LoadModel();
    //VRE::VRE_GameObject& obj = mGameObjectManager.CreateGameObject();
    //obj.mModel = model;
    //obj.mTransform.mTranslation = { 0.f, -.5f, 0.f };
    //obj.mTransform.mRotation = { 0.f, glm::pi<float>(), 0.f };
    //obj.mTransform.mScale = { 0.01f, 0.01f, 0.01f };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //std::shared_ptr<VRE_glTFModel> model = std::make_shared<VRE_glTFModel>(mDevice, "Resources/Models/ToyCar/", "ToyCar");
    //model->LoadModel();
    //VRE::VRE_GameObject& obj = mGameObjectManager.CreateGameObject();
    //obj.mModel = model;
    //obj.mTransform.mTranslation = { 0.f, -.25f, 0.f };
    //obj.mTransform.mRotation = { glm::half_pi<float>(), glm::pi<float>(), 0.f };
    //obj.mTransform.mScale = { 0.005f, 0.005f, 0.005f };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //std::shared_ptr<VRE_glTFModel> model = std::make_shared<VRE_glTFModel>(mDevice, "Resources/Models/FlightHelmet/", "FlightHelmet");
    //model->LoadImages();
    //VRE::VRE_GameObject& obj = mGameObjectManager.CreateGameObject();
    //obj.mModel = model;
    //obj.mTransform.mTranslation = { 0.f, -.5f, 0.f };
    //obj.mTransform.mRotation = { 0.f, glm::pi<float>(), 0.f };
    //obj.mTransform.mScale = { 3.0f, 3.0f, 3.0f };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<VRE_glTFModel> model = std::make_shared<VRE_glTFModel>(mDevice, "Resources/Models/DamagedHelmet/", "DamagedHelmet");
    model->LoadModel();
    VRE::VRE_GameObject& obj = mGameObjectManager.CreateGameObject();
    obj.mModel = model;
    obj.mTransform.mTranslation = { 0.f, -.5f, 0.f };
    obj.mTransform.mRotation = { glm::half_pi<float>(), glm::pi<float>(), 0.f };
    obj.mTransform.mScale = { 1.0f, 1.0f, 1.0f };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<glm::vec3> coloredLights{{1.f, .1f, .1f},
                                         {.1f, .1f, 1.f},
                                         {.1f, 1.f, .1f},
                                         {1.f, 1.f, .1f},
                                         {.1f, 1.f, 1.f},
                                         {1.f, 1.f, 1.f}};

    for (int i = 0; i < coloredLights.size(); i++) {
        auto pointLight = VRE_PointLight::CreatePointLight(0.75f);
        pointLight.mColor = coloredLights[i];
        auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / coloredLights.size(), { 0.f, 1.f, 0.f });
        pointLight.mPosition = rotateLight * glm::vec4(-1.f, 1.f, -1.f, 1.f);
        mSceneContext.mPointLights.push_back(std::move(pointLight));
    }
}
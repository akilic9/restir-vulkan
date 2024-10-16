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

#include "VRE_App.h"
#include "VRE_RenderSystem.h"
#include "VRE_PointLightRenderSystem.h"
#include "VRE_Camera.h"
#include "VRE_InputListener.h"
#include "VRE_Buffer.h"
#include "VRE_glTFModel.h"
#include <iostream>
#include <array>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/constants.hpp>

// TinyGLTF library definitions and include.
// Example and instructions: https://github.com/syoyo/tinygltf?tab=readme-ov-file#loading-gltf-20-model
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <tiny_gltf.h>

VRE::VRE_App::VRE_App() : mRenderer(mWindow, mDevice), mGameObjectManager(mDevice)
{
    mDescriptorPool = VRE_DescriptorPool::Builder(mDevice)
                      .SetMaxSets(VRE_SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VRE_SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .Build();

    mFramePools.resize(VRE_SwapChain::MAX_FRAMES_IN_FLIGHT);
    auto framePoolBuilder = VRE_DescriptorPool::Builder(mDevice)
        .SetMaxSets(1000)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
        .SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    for (int i = 0; i < mFramePools.size(); i++) {
        mFramePools[i] = framePoolBuilder.Build();
    }

    LoadObjects();
}

VRE::VRE_App::~VRE_App() {}

void VRE::VRE_App::Run()
{
    std::vector<std::unique_ptr<VRE_Buffer>> uboBuffers(VRE_SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<VRE_Buffer>(mDevice, sizeof(UBO), 1,
                                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->Map();
    }

    auto descSetLayout = VRE_DescriptorSetLayout::Builder(mDevice)
                         .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                         .Build();


    std::vector<VkDescriptorSet> descriptorSets(VRE_SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < descriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->DescriptorInfo();
        VRE_DescriptorWriter(*descSetLayout, *mDescriptorPool)
                            .WriteBuffer(0, &bufferInfo)
                            .Build(descriptorSets[i]);
    }

    VRE_RenderSystem renderSys(mDevice, mRenderer.GetSwapChainRenderPass(), descSetLayout->GetDescriptorSetLayout());
    VRE_PointLightRenderSystem lightRenderSys(mDevice, mRenderer.GetSwapChainRenderPass(), descSetLayout->GetDescriptorSetLayout());
    VRE_Camera camera{};
    VRE_InputListener inputListener{};

    auto startTime = std::chrono::high_resolution_clock::now();

    while (!mWindow.ShouldClose()) {
        glfwPollEvents();

        const auto currentTime = std::chrono::high_resolution_clock::now();
        const float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        startTime = currentTime;

        float aspRatio = mRenderer.GetAspectRatio();
        camera.SetPerspectiveProjection(glm::radians(50.f), aspRatio, 0.1f, 1000.f);

        // TODO: can probably optimize this using glfw callback function.
        inputListener.Move(mWindow.GetGLFWwindow(), deltaTime, camera);

        if (auto commandBuffer = mRenderer.BeginDraw()) {
            const int frameIndex = mRenderer.GetFrameIndex();
            mFramePools[frameIndex]->ResetPool();
            VRE_FrameInfo frameInfo{ frameIndex, commandBuffer, camera, descriptorSets[frameIndex],
                                     *mFramePools[frameIndex], mGameObjectManager.mGameObjectsMap, mPointLights};

            //Update
            UBO ubo;
            ubo.mProjectionMat = camera.GetProjection();
            ubo.mViewMat = camera.GetViewMat();
            ubo.mInvViewMat = camera.GetInvViewMat();
            lightRenderSys.Update(frameInfo, ubo, deltaTime);
            uboBuffers[frameIndex]->WriteToBuffer(&ubo);
            uboBuffers[frameIndex]->Flush();

            mGameObjectManager.UpdateBuffer(frameIndex);

            //Render
            mRenderer.BeginSwapChainRenderPass(commandBuffer);
            renderSys.RenderGameObjects(frameInfo);
            lightRenderSys.RenderLights(frameInfo);
            mRenderer.EndSwapChainRenderPass(commandBuffer);
            mRenderer.EndDraw();
        }
    }
    vkDeviceWaitIdle(mDevice.GetVkDevice());
}

void VRE::VRE_App::LoadObjects()
{
    std::shared_ptr<VRE_glTFModel> model = std::make_shared<VRE_glTFModel>(mDevice, "Resources/Models/FlightHelmet/", "FlightHelmet");
    model->LoadImages();
    VRE::VRE_GameObject& duck = mGameObjectManager.CreateGameObject();
    duck.mModel = model;
    duck.mTransform.mTranslation = { 0.f, 0.f, 0.f };
    duck.mTransform.mScale = { 1.f, 1.f, 1.f };

    //std::shared_ptr<VRE_Model> model = VRE_Model::CreateModel(mDevice, "Resources/Models/flat_vase.obj");
    //auto flatVase = VRE_GameObject::CreateGameObject();
    //flatVase.mModel = model;
    //flatVase.mTransform.mTranslation = { -.5f, .5f, 0.f };
    //flatVase.mTransform.mScale = { 3.f, 1.5f, 3.f };
    //mGameObjects.emplace(flatVase.GetID(), std::move(flatVase));

    //model = VRE_Model::CreateModel(mDevice, "Resources/Models/smooth_vase.obj");
    //auto smoothVase = VRE_GameObject::CreateGameObject();
    //smoothVase.mModel = model;
    //smoothVase.mTransform.mTranslation = { .5f, .5f, 0.f };
    //smoothVase.mTransform.mScale = { 3.f, 1.5f, 3.f };
    //mGameObjects.emplace(smoothVase.GetID(), std::move(smoothVase));

    //model = VRE_Model::CreateModel(mDevice, "Resources/Models/quad.obj");
    //auto floor = VRE_GameObject::CreateGameObject();
    //floor.mModel = model;
    //floor.mTransform.mTranslation = { 0.f, .5f, 0.f };
    //floor.mTransform.mScale = { 3.f, 1.f, 3.f };
    //mGameObjects.emplace(floor.GetID(), std::move(floor));
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    //mTexture = VRE_Texture::CreateTexture(mDevice, "Resources/Textures/viking_room.png");

    //std::shared_ptr<VRE_Model> model = VRE_Model::CreateModel(mDevice, "Resources/Models/viking_room.obj");
    //auto room = VRE_GameObject::CreateGameObject();
    //room.mModel = model;
    //room.mTransform.mRotation = { glm::half_pi<float>(), glm::half_pi<float>(), 0.f };
    //room.mTransform.mTranslation = { 0.f, 0.f, 0.f };
    //room.mTransform.mScale = { 1.f, 1.f, 1.f };
    //mGameObjects.emplace(room.GetID(), std::move(room));

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    //mTexture = VRE_Texture::CreateTexture(mDevice, "Resources/Textures/gold.jpg");

    //std::shared_ptr<VRE_Model> model = VRE_Model::CreateModel(mDevice, "Resources/Models/coin-gold.obj");
    //auto coin = VRE_GameObject::CreateGameObject();
    //coin.mModel = model;
    //coin.mTransform.mTranslation = { .0f, -.5f, 0.5f };
    //coin.mTransform.mScale = { 1.5f, 1.5f, 1.5f };
    //mGameObjects.emplace(coin.GetID(), std::move(coin));

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<glm::vec3> coloredLights{{1.f, .1f, .1f},
                                         {.1f, .1f, 1.f},
                                         {.1f, 1.f, .1f},
                                         {1.f, 1.f, .1f},
                                         {.1f, 1.f, 1.f},
                                         {1.f, 1.f, 1.f}};

    for (int i = 0; i < coloredLights.size(); i++) {
        auto pointLight = VRE_PointLight::CreatePointLight(0.2f);
        pointLight.mColor = coloredLights[i];
        auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / coloredLights.size(), { 0.f, 1.f, 0.f });
        pointLight.mPosition = rotateLight * glm::vec4(-1.f, 1.f, -1.f, 1.f);
        mPointLights.push_back(std::move(pointLight));
    }

    //auto pointLight = VRE_PointLight::CreatePointLight(0.5f);
    //pointLight.mColor = { 0.5f, 0.5f, 0.5f };
    //pointLight.mPosition = { 0.f, 0.f, 0.f, 1.f };
    //pointLight.mScale = 0.2f;
    //mPointLights.push_back(std::move(pointLight));
}
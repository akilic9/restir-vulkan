#include "VRE_App.h"
#include "VRE_RenderSystem.h"
#include "VRE_PointLightRenderSystem.h"
#include "VRE_Camera.h"
#include "VRE_InputListener.h"
#include "VRE_Buffer.h"
#include <iostream>
#include <array>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/constants.hpp>

namespace VRE {
    struct UBO
    {
        glm::mat4 mProjectionMat = 1.f;
        glm::mat4 mViewMat = 1.f;
        glm::vec4 mAmbientLightColor{1.f, 1.f, 1.f, 0.02f}; // r, g, b, intensity
        glm::vec3 mLightPosition{-1.f};
        alignas(16) glm::vec4 mLightColor{1.f}; // r, g, b, intensity
    };
}

VRE::VRE_App::VRE_App()
    : mRenderer{mWindow, mDevice}
{
    mDescriptorPool = VRE_DescriptorPool::Builder(mDevice)
                      .SetMaxSets(VRE_SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VRE_SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .Build();

    LoadGameObjects();
}

VRE::VRE_App::~VRE_App() {}

void VRE::VRE_App::Run()
{
    std::vector<std::unique_ptr<VRE_Buffer>> uboBuffers(VRE_SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<VRE_Buffer>(
            mDevice,
            sizeof(UBO),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->Map();
    }

    VRE_Buffer UBOBuffer(mDevice, sizeof(UBO), VRE_SwapChain::MAX_FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, mDevice.properties.limits.minUniformBufferOffsetAlignment);

    UBOBuffer.Map();

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

    //camera.SetViewDirection(glm::vec3{ 0.f }, glm::vec3{ 0.5f, 0.f, 1.f });
    //camera.SetViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!mWindow.ShouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        // TODO: can probably optimize this using glfw callback function.
        inputListener.Move(mWindow.GetGLFWwindow(), deltaTime, camera);

        float aspRatio = mRenderer.GetAspectRatio();
        //camera.SetOrthographicProjection(-1.f, 1.f, -aspRatio, aspRatio, -1.f, 1.f);
        camera.SetPerspectiveProjection(glm::radians(50.f), aspRatio, 0.1f, 1000.f);

        if (auto commandBuffer = mRenderer.BeginDraw()) {
            int frameIndex = mRenderer.GetFrameIndex();
            VRE_FrameInfo frameInfo{ frameIndex, deltaTime, commandBuffer, camera, descriptorSets[frameIndex], mGameObjects};

            //update
            UBO ubo;
            ubo.mProjectionMat = camera.GetProjection();
            ubo.mViewMat = camera.GetViewMat();
            uboBuffers[frameIndex]->WriteToBuffer(&ubo);
            uboBuffers[frameIndex]->Flush();

            //render
            mRenderer.BeginSwapChainRenderPass(commandBuffer);
            renderSys.RenderGameObjects(frameInfo);
            lightRenderSys.RenderLights(frameInfo);
            mRenderer.EndSwapChainRenderPass(commandBuffer);
            mRenderer.EndDraw();
        }
    }
    vkDeviceWaitIdle(mDevice.device());
}

void VRE::VRE_App::LoadGameObjects()
{
    std::shared_ptr<VRE_Model> model = VRE_Model::CreateModel(mDevice, "Resources/Models/flat_vase.obj");
    auto flatVase = VRE_GameObject::CreateGameObject();
    flatVase.mModel = model;
    flatVase.mTransform.mTranslation = { -.5f, .5f, 0.f };
    flatVase.mTransform.mScale = { 3.f, 1.5f, 3.f };
    mGameObjects.emplace(flatVase.GetID(), std::move(flatVase));

    model = VRE_Model::CreateModel(mDevice, "Resources/Models/smooth_vase.obj");
    auto smoothVase = VRE_GameObject::CreateGameObject();
    smoothVase.mModel = model;
    smoothVase.mTransform.mTranslation = { .5f, .5f, 0.f };
    smoothVase.mTransform.mScale = { 3.f, 1.5f, 3.f };
    mGameObjects.emplace(smoothVase.GetID(), std::move(smoothVase));

    model = VRE_Model::CreateModel(mDevice, "Resources/Models/quad.obj");
    auto floor = VRE_GameObject::CreateGameObject();
    floor.mModel = model;
    floor.mTransform.mTranslation = { 0.f, .5f, 0.f };
    floor.mTransform.mScale = { 3.f, 1.f, 3.f };
    mGameObjects.emplace(floor.GetID(), std::move(floor));

    //std::shared_ptr<VRE_Model> model = VRE::VRE_Model::CreateModel(mDevice, "Resources/Models/crate-strong.obj");

    //auto obj = VRE::VRE_GameObject::CreateGameObject();
    //obj.mModel = model;
    //obj.mTransform.mTranslation = { 0.f, 0.f, 0.f };
    //obj.mTransform.mScale = glm::vec3{ 1.f };

    //mGameObjects.emplace(obj.GetID(), std::move(obj));
}
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

VRE::VRE_App::VRE_App() : mRenderer{mWindow, mDevice}
{
    mDescriptorPool = VRE_DescriptorPool::Builder(mDevice)
                      .SetMaxSets(VRE_SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VRE_SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VRE_SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .Build();

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
                         .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                         .Build();

    auto imageInfo = mTexture->getImageInfo();


    std::vector<VkDescriptorSet> descriptorSets(VRE_SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < descriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->DescriptorInfo();
        VRE_DescriptorWriter(*descSetLayout, *mDescriptorPool)
                            .WriteBuffer(0, &bufferInfo)
                            .WriteImage(1, &imageInfo)
                            .Build(descriptorSets[i]);
    }

    VRE_RenderSystem renderSys(mDevice, mRenderer.GetSwapChainRenderPass(), descSetLayout->GetDescriptorSetLayout());
    VRE_PointLightRenderSystem lightRenderSys(mDevice, mRenderer.GetSwapChainRenderPass(), descSetLayout->GetDescriptorSetLayout());
    VRE_Camera camera{};
    VRE_InputListener inputListener{};

    //camera.SetViewDirection(glm::vec3{ 0.f }, glm::vec3{ 0.5f, 0.f, 1.f });
    //camera.SetViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));

    auto startTime = std::chrono::high_resolution_clock::now();

    while (!mWindow.ShouldClose()) {
        glfwPollEvents();

        const auto currentTime = std::chrono::high_resolution_clock::now();
        const float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        startTime = currentTime;

        // TODO: can probably optimize this using glfw callback function.
        inputListener.Move(mWindow.GetGLFWwindow(), deltaTime, camera);

        float aspRatio = mRenderer.GetAspectRatio();
        //camera.SetOrthographicProjection(-1.f, 1.f, -aspRatio, aspRatio, -1.f, 1.f);
        camera.SetPerspectiveProjection(glm::radians(50.f), aspRatio, 0.1f, 1000.f);

        if (auto commandBuffer = mRenderer.BeginDraw()) {
            const int frameIndex = mRenderer.GetFrameIndex();
            VRE_FrameInfo frameInfo{ frameIndex, commandBuffer, camera, descriptorSets[frameIndex], mGameObjects, mPointLights};

            //Update
            UBO ubo;
            ubo.mProjectionMat = camera.GetProjection();
            ubo.mViewMat = camera.GetViewMat();
            ubo.mInvViewMat = camera.GetInvViewMat();
            lightRenderSys.Update(frameInfo, ubo, deltaTime);
            uboBuffers[frameIndex]->WriteToBuffer(&ubo);
            uboBuffers[frameIndex]->Flush();

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
    mTexture = VRE_Texture::CreateTexture(mDevice, "Resources/Models/test-text.jpg");

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

    std::vector<glm::vec3> coloredLights{{1.f, .1f, .1f},
                                         {.1f, .1f, 1.f},
                                         {.1f, 1.f, .1f},
                                         {1.f, 1.f, .1f},
                                         {.1f, 1.f, 1.f},
                                         {1.f, 1.f, 1.f}};

    for (int i = 0; i < coloredLights.size(); i++) {
        auto pointLight = VRE_PointLight::CreatePointLight(0.2f);
        pointLight.mColor = coloredLights[i];
        auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / coloredLights.size(), { 0.f, -1.f, 0.f });
        pointLight.mPosition = rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f);
        mPointLights.push_back(std::move(pointLight));
    }
}
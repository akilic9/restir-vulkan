#include "VRE_App.h"
#include "VRE_RenderSystem.h"
#include <iostream>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/constants.hpp>

VRE::VRE_App::VRE_App()
    : mRenderer{mWindow, mDevice}
{
    LoadGameObjects();
}

VRE::VRE_App::~VRE_App() {}

void VRE::VRE_App::Run()
{
    VRE_RenderSystem renderSys(mDevice, mRenderer.GetSwapChainRenderPass());
    while (!mWindow.ShouldClose()) {
        glfwPollEvents();

        if (auto commandBuffer = mRenderer.BeginDraw()) {
            mRenderer.BeginSwapChainRenderPass(commandBuffer);
            renderSys.RenderGameObjects(commandBuffer, mGameObjects);
            mRenderer.EndSwapChainRenderPass(commandBuffer);
            mRenderer.EndDraw();
        }
    }
    vkDeviceWaitIdle(mDevice.device());
}

void VRE::VRE_App::LoadGameObjects()
{
    std::vector<VRE_Model::Vertex> vertices{
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}} };

    auto model = std::make_shared<VRE_Model>(mDevice, vertices);

    auto triangle = VRE::VRE_GameObject::CreateGameObject();
    triangle.mModel = model;
    triangle.mColor = { .1f, .8f, .1f };
    triangle.mTransform.position.x = .2f;
    triangle.mTransform.scale = { 2.f, .5f };
    triangle.mTransform.rotation = 0.25f * glm::two_pi<float>();

    mGameObjects.push_back(std::move(triangle));
}
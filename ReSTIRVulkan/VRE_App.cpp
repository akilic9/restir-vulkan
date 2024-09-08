#include "VRE_App.h"
#include "VRE_RenderSystem.h"
#include "VRE_Camera.h"
#include "VRE_InputListener.h"
#include <iostream>
#include <array>
#include <chrono>

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
    VRE_Camera camera{};
    VRE_InputListener inputListener{};

    //camera.SetViewDirection(glm::vec3{ 0.f }, glm::vec3{ 0.5f, 0.f, 1.f });
    camera.SetViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!mWindow.ShouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        // TODO: can probably optimize this using glfw callback function,
        inputListener.Move(mWindow.GetGLFWwindow(), deltaTime, camera);

        float aspRatio = mRenderer.GetAspectRatio();
        //camera.SetOrthographicProjection(-1.f, 1.f, -aspRatio, aspRatio, -1.f, 1.f);
        camera.SetPerspectiveProjection(glm::radians(50.f), aspRatio, 0.1f, 10.f);

        if (auto commandBuffer = mRenderer.BeginDraw()) {
            mRenderer.BeginSwapChainRenderPass(commandBuffer);
            renderSys.RenderGameObjects(commandBuffer, mGameObjects, camera);
            mRenderer.EndSwapChainRenderPass(commandBuffer);
            mRenderer.EndDraw();
        }
    }
    vkDeviceWaitIdle(mDevice.device());
}

void VRE::VRE_App::LoadGameObjects()
{
    std::shared_ptr<VRE_Model> model = VRE::VRE_Model::CreateModel(mDevice, "Resources/Models/crate-strong.obj");

    auto obj = VRE::VRE_GameObject::CreateGameObject();
    obj.mModel = model;
    obj.mColor = { .1f, .8f, .1f };
    obj.mTransform.translation = { 0.f, 0.f, 2.5f };
    obj.mTransform.scale = { 0.5f, 0.5f, 0.5f };

    mGameObjects.push_back(std::move(obj));
}
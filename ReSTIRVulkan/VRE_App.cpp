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

std::unique_ptr<VRE::VRE_Model> VRE::VRE_App::CreateCubeModel(VRE_Device& device, glm::vec3 offset)
{
    VRE_Model::ModelData modelData{};
    modelData.mVertices = {
        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
    };
    for (auto& v : modelData.mVertices) {
        v.position += offset;
    }

    modelData.mIndices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                            12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

    return std::make_unique<VRE_Model>(device, modelData);
}

void VRE::VRE_App::LoadGameObjects()
{
    std::shared_ptr<VRE_Model> model = CreateCubeModel(mDevice, glm::vec3{ 0.f });

    auto cube = VRE::VRE_GameObject::CreateGameObject();
    cube.mModel = model;
    cube.mColor = { .1f, .8f, .1f };
    cube.mTransform.translation = { 0.f, 0.f, 2.5f };
    cube.mTransform.scale = { 0.5f, 0.5f, 0.5f };

    mGameObjects.push_back(std::move(cube));
}
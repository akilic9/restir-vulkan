#include "VRE_App.h"
#include "VRE_RenderSystem.h"
#include "VRE_Camera.h"
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
    VRE_Camera camera{};
    //camera.SetViewDirection(glm::vec3{ 0.f }, glm::vec3{ 0.5f, 0.f, 1.f });
    camera.SetViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));


    while (!mWindow.ShouldClose()) {
        glfwPollEvents();

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
    std::vector<VRE_Model::Vertex> vertices{

        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

    };
    for (auto& v : vertices) {
        v.position += offset;
    }
    return std::make_unique<VRE_Model>(device, vertices);
}

void VRE::VRE_App::LoadGameObjects()
{
    std::shared_ptr<VRE_Model> model = CreateCubeModel(mDevice, glm::vec3{ 0.f });

    auto cube = VRE::VRE_GameObject::CreateGameObject();
    cube.mModel = model;
    cube.mColor = { .1f, .8f, .1f };
    cube.mTransform.translation = {0.f, 0.f, 2.5f};
    cube.mTransform.scale = { 0.5f, 0.5f, 0.5f };

    mGameObjects.push_back(std::move(cube));
}
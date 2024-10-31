/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube.
*       Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*/
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/constants.hpp>

namespace VRE {
    class VRE_Camera
    {
    public:
        void SetOrthographicProjection(float top, float bottom, float left, float right, float near, float far);
        void SetPerspectiveProjection(float fovY, float aspectRatio, float near, float far);

        void SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
        void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
        void SetViewXYZ(glm::vec3 position, glm::vec3 rotation);

        const glm::mat4 GetProjection() const { return mProjectionMatrix; }
        const glm::mat4 GetViewMat() const { return mViewMatrix; }
        const glm::mat4 GetInvViewMat() const { return mInvViewMatrix; }

        glm::vec3 GetRotation() const { return mRotation; }
        glm::vec3 GetPosition() const { return mPosition; }

    private:
        glm::vec3 mPosition{ 0.f, 0.f, -5.f };
        glm::vec3 mRotation{ 0.f, 0.f, glm::pi<float>() };
        glm::mat4 mProjectionMatrix{ 1.f };
        glm::mat4 mViewMatrix{ 1.f };
        glm::mat4 mInvViewMatrix{ 1.f };
    };
}
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

namespace VRE {
    class VRE_Camera
    {
    public:
        void SetOrthographicProjection(float top, float bottom, float left, float right, float near, float far);
        void SetPerspectiveProjection(float fovY, float aspectRatio, float near, float far);

        void SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.f, -1.f, 0.f));
        void SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3(0.f, -1.f, 0.f));
        void SetViewXYZ(glm::vec3 position, glm::vec3 rotation);

        const glm::mat4 GetProjection() const { return mProjectionMatrix; }
        const glm::mat4 GetViewMat() const { return mViewMatrix; }
    private:
        glm::mat4 mProjectionMatrix{ 1.f };
        glm::mat4 mViewMatrix{ 1.f };
    };
}
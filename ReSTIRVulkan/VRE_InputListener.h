#pragma once
#include "VRE_Camera.h"
#include "VRE_Window.h"

namespace VRE {
    class VRE_InputListener
    {
    public:
        void Init(GLFWwindow* window);
        void Move(GLFWwindow* window, float dt, VRE_Camera &camera);

        float mMoveSpeed{ 2.f };
        float mMouseSensitivity{ 0.1f };
    };
}


#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
//Window wrapper class.

namespace VRE {
    class VRE_Window
    {
    public:
        VRE_Window(int width, int height, std::string name);
        ~VRE_Window();

        VRE_Window(const VRE_Window&) = delete;
        VRE_Window &operator=(const VRE_Window&) = delete;

        inline bool ShouldClose() { return glfwWindowShouldClose(mWindow); }

        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

        inline VkExtent2D GetExtent() { return { static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight) }; }
        inline bool HasWindowResized() { return mFrameBufferResized; }
        inline void ResetWindowResizedFlag() { mFrameBufferResized = false; }

    private:
        void Init();
        static void FrameBufferResizedCallback(GLFWwindow* window, int width, int height);

        int mWidth;
        int mHeight;
        bool mFrameBufferResized = false;
        const std::string mWindowName;

        GLFWwindow* mWindow;
    };
}



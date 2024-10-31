/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*   Overvoorde, A. (2017). Khronos Vulkan Tutorial. [online] Vulkan.org. Available at: https://docs.vulkan.org/tutorial/latest/00_Introduction.html (Accessed 07 June 2024).
*/
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
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
        GLFWwindow* GetGLFWwindow() const { return mWindow; }

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
/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*   Overvoorde, A. (2017). Khronos Vulkan Tutorial. [online] Vulkan.org. Available at: https://docs.vulkan.org/tutorial/latest/00_Introduction.html (Accessed 07 June 2024).
*/
#include "VRE_Window.h"
#include <iostream>

VRE::VRE_Window::VRE_Window(int width, int height, std::string name)
    : mWidth(width)
    , mHeight(height)
    , mWindowName(name)
    , mWindow(nullptr)
{
    Init();
}

VRE::VRE_Window::~VRE_Window()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void VRE::VRE_Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (glfwCreateWindowSurface(instance, mWindow, nullptr, surface) != VK_SUCCESS)
        std::cout << "Cannot create window surface!" << std::endl;
}

void VRE::VRE_Window::Init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    mWindow = glfwCreateWindow(mWidth, mHeight, mWindowName.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, FrameBufferResizedCallback);
}

void VRE::VRE_Window::FrameBufferResizedCallback(GLFWwindow* window, int width, int height)
{
    auto win = reinterpret_cast<VRE_Window*>(glfwGetWindowUserPointer(window));
    win->mFrameBufferResized = true;
    win->mWidth = width;
    win->mHeight = height;
}
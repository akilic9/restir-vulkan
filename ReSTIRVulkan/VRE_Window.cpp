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

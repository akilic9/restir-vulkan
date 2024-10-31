/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   OpenGL Game Engine Template - Camera class, INM376 Computer Graphics module, City St Georges University of London.
*/

#include "VRE_InputListener.h"
#include <iostream>

// TODO: Expand this into a proper input system.
void VRE::VRE_InputListener::Move(GLFWwindow* window, float dt, VRE_Camera& camera)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    int middleX = width >> 1;
    int middleY = height >> 1;

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    double rotX = camera.GetRotation().y + (middleX - mouseX) * mMouseSensitivity;
    double rotY = camera.GetRotation().x + (mouseY - middleY) * mMouseSensitivity;

    glm::vec3 rotation{ glm::radians(rotY), glm::radians(rotX), camera.GetRotation().z };
    rotation.x = glm::clamp(rotation.x, -1.5f, 1.5f);

    float yaw = camera.GetRotation().y;
    float pitch = camera.GetRotation().x;
    const glm::vec3 forwardDir{ sin(yaw), -sin(rotation.x), cos(yaw)};
    const glm::vec3 rightDir{ -forwardDir.z, 0.f, forwardDir.x };
    const glm::vec3 upDir{ 0.f, 1.f, 0.f };

    glm::vec3 moveDir{ 0.f };
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += forwardDir;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= forwardDir;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += rightDir;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= rightDir;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) moveDir += upDir;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) moveDir -= upDir;

    glm::vec3 position = camera.GetPosition();

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        position += mMoveSpeed * dt * glm::normalize(moveDir);

    camera.SetViewXYZ(position, rotation);
}
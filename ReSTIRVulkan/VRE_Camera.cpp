#include "VRE_Camera.h"
#include <cassert>
#include <limits>

void VRE::VRE_Camera::SetOrthographicProjection(float top, float bottom, float left, float right, float near, float far)
{
    mProjectionMatrix = glm::mat4{ 1.0f };
    mProjectionMatrix[0][0] = 2.f / (right - left);
    mProjectionMatrix[1][1] = 2.f / (bottom - top);
    mProjectionMatrix[2][2] = 1.f / (far - near);
    mProjectionMatrix[3][0] = -(right + left) / (right - left);
    mProjectionMatrix[3][1] = -(bottom + top) / (bottom - top);
    mProjectionMatrix[3][2] = -near / (far - near);
}

void VRE::VRE_Camera::SetPerspectiveProjection(float fovY, float aspectRatio, float near, float far)
{
    assert(glm::abs(aspectRatio - std::numeric_limits<float>::epsilon()) > 0.0f);
    const float tanHalfFovY = tan(fovY / 2.f);
    mProjectionMatrix = glm::mat4{ 0.0f };
    mProjectionMatrix[0][0] = 1.f / (aspectRatio * tanHalfFovY);
    mProjectionMatrix[1][1] = 1.f / (tanHalfFovY);
    mProjectionMatrix[2][2] = far / (far - near);
    mProjectionMatrix[2][3] = 1.f;
    mProjectionMatrix[3][2] = -(far * near) / (far - near);
}

void VRE::VRE_Camera::SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
    mPosition = position;
    const glm::vec3 w{ glm::normalize(direction) };
    const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
    const glm::vec3 v{ glm::cross(w, u) };

    mViewMatrix = glm::mat4{ 1.f };
    mViewMatrix[0][0] = u.x;
    mViewMatrix[1][0] = u.y;
    mViewMatrix[2][0] = u.z;
    mViewMatrix[0][1] = v.x;
    mViewMatrix[1][1] = v.y;
    mViewMatrix[2][1] = v.z;
    mViewMatrix[0][2] = w.x;
    mViewMatrix[1][2] = w.y;
    mViewMatrix[2][2] = w.z;
    mViewMatrix[3][0] = -glm::dot(u, position);
    mViewMatrix[3][1] = -glm::dot(v, position);
    mViewMatrix[3][2] = -glm::dot(w, position);

    mInvViewMatrix = glm::mat4{ 1.f };
    mInvViewMatrix[0][0] = u.x;
    mInvViewMatrix[0][1] = u.y;
    mInvViewMatrix[0][2] = u.z;
    mInvViewMatrix[1][0] = v.x;
    mInvViewMatrix[1][1] = v.y;
    mInvViewMatrix[1][2] = v.z;
    mInvViewMatrix[2][0] = w.x;
    mInvViewMatrix[2][1] = w.y;
    mInvViewMatrix[2][2] = w.z;
    mInvViewMatrix[3][0] = position.x;
    mInvViewMatrix[3][1] = position.y;
    mInvViewMatrix[3][2] = position.z;
}

void VRE::VRE_Camera::SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
    SetViewDirection(position, target - position, up);
}

void VRE::VRE_Camera::SetViewXYZ(glm::vec3 position, glm::vec3 rotation)
{
    mPosition = position;
    mRotation = rotation;
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
    const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
    const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
    mViewMatrix = glm::mat4{ 1.f };
    mViewMatrix[0][0] = u.x;
    mViewMatrix[1][0] = u.y;
    mViewMatrix[2][0] = u.z;
    mViewMatrix[0][1] = v.x;
    mViewMatrix[1][1] = v.y;
    mViewMatrix[2][1] = v.z;
    mViewMatrix[0][2] = w.x;
    mViewMatrix[1][2] = w.y;
    mViewMatrix[2][2] = w.z;
    mViewMatrix[3][0] = -glm::dot(u, position);
    mViewMatrix[3][1] = -glm::dot(v, position);
    mViewMatrix[3][2] = -glm::dot(w, position);

    mInvViewMatrix = glm::mat4{ 1.f };
    mInvViewMatrix[0][0] = u.x;
    mInvViewMatrix[0][1] = u.y;
    mInvViewMatrix[0][2] = u.z;
    mInvViewMatrix[1][0] = v.x;
    mInvViewMatrix[1][1] = v.y;
    mInvViewMatrix[1][2] = v.z;
    mInvViewMatrix[2][0] = w.x;
    mInvViewMatrix[2][1] = w.y;
    mInvViewMatrix[2][2] = w.z;
    mInvViewMatrix[3][0] = position.x;
    mInvViewMatrix[3][1] = position.y;
    mInvViewMatrix[3][2] = position.z;
}
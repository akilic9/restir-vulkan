//Transform code by Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube.
// Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
#pragma once
#include "glm.hpp"

namespace VRE {
    static const int MAX_OBJECT_COUNT = 1000;

    struct Transform {
        glm::vec3 mTranslation{ 0.f };
        glm::vec3 mScale{ 1.f };
        glm::vec3 mRotation{ 0.f };

        // Matrix corresponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // Wikipedia Contributors (2024). Euler angles. [online] Wikipedia. Available at: https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix.
        glm::mat4 Mat4() {
            const float c3 = glm::cos(mRotation.z);
            const float s3 = glm::sin(mRotation.z);
            const float c2 = glm::cos(mRotation.x);
            const float s2 = glm::sin(mRotation.x);
            const float c1 = glm::cos(mRotation.y);
            const float s1 = glm::sin(mRotation.y);
            return glm::mat4{
                {
                    mScale.x * (c1 * c3 + s1 * s2 * s3),
                    mScale.x * (c2 * s3),
                    mScale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    mScale.y * (c3 * s1 * s2 - c1 * s3),
                    mScale.y * (c2 * c3),
                    mScale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    mScale.z * (c2 * s1),
                    mScale.z * (-s2),
                    mScale.z * (c1 * c2),
                    0.0f,
                },
                {mTranslation.x, mTranslation.y, mTranslation.z, 1.0f} };
        }

        glm::mat3 NormalMatrix() {
            const float c3 = glm::cos(mRotation.z);
            const float s3 = glm::sin(mRotation.z);
            const float c2 = glm::cos(mRotation.x);
            const float s2 = glm::sin(mRotation.x);
            const float c1 = glm::cos(mRotation.y);
            const float s1 = glm::sin(mRotation.y);

            const glm::vec3 inverseScale = 1.0f / mScale;

            return glm::mat3{
                {
                    inverseScale.x * (c1 * c3 + s1 * s2 * s3),
                    inverseScale.x * (c2 * s3),
                    inverseScale.x * (c1 * s2 * s3 - c3 * s1),
                },
                {
                    inverseScale.y * (c3 * s1 * s2 - c1 * s3),
                    inverseScale.y * (c2 * c3),
                    inverseScale.y * (c1 * c3 * s2 + s1 * s3),
                },
                {
                    inverseScale.z * (c2 * s1),
                    inverseScale.z * (-s2),
                    inverseScale.z * (c1 * c2),
                } };
        }
    };
}
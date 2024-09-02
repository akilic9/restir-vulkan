#pragma once
#include <memory>
#include "VRE_Model.h"
#include "gtc/matrix_transform.hpp"

namespace VRE {

    struct Transform {
        glm::vec3 translation{ 0.f };
        glm::vec3 scale{ 1.f };
        glm::vec3 rotation{0.f};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4() {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            return glm::mat4{
                {
                    scale.x * (c1 * c3 + s1 * s2 * s3),
                    scale.x * (c2 * s3),
                    scale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    scale.y * (c3 * s1 * s2 - c1 * s3),
                    scale.y * (c2 * c3),
                    scale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    scale.z * (c2 * s1),
                    scale.z * (-s2),
                    scale.z * (c1 * c2),
                    0.0f,
                },
                {translation.x, translation.y, translation.z, 1.0f} };
        }
    };

    class VRE_GameObject
    {
    public:
        using objectID = unsigned int;

        static VRE_GameObject CreateGameObject() {
            static objectID currentID = 0;
            return VRE_GameObject(currentID++);
        }

        VRE_GameObject(const VRE_GameObject&) = delete;
        VRE_GameObject& operator=(const VRE_GameObject&) = delete;
        VRE_GameObject(VRE_GameObject&&) = default;
        VRE_GameObject& operator=(VRE_GameObject&&) = default;

        objectID GetID() const { return mID; }

        std::shared_ptr<VRE_Model> mModel;
        glm::vec3 mColor;
        Transform mTransform{};

    private:
        VRE_GameObject(objectID id) : mID(id), mColor(1.f) {};

        objectID mID;
    };
}
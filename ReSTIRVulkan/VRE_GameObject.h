#pragma once
#include <memory>
#include <unordered_map>
#include "VRE_Model.h"
#include "gtc/matrix_transform.hpp"

namespace VRE {

    struct Transform {
        glm::vec3 mTranslation{ 0.f };
        glm::vec3 mScale{ 1.f };
        glm::vec3 mRotation{ 0.f };

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 Mat4();
        glm::mat3 NormalMatrix();
    };

    class VRE_GameObject
    {
    public:
        using objectID = unsigned int;
        using ObjectsMap = std::unordered_map<objectID, VRE_GameObject>;

        VRE_GameObject(objectID id);

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
        objectID mID;
    };
}
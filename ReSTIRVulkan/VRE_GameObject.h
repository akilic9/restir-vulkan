#pragma once
#include <memory>
#include "VRE_Model.h"

namespace VRE {

    struct Transform2D {
        glm::vec2 position{0.f};
        glm::vec2 scale{1.f};
        float rotation = 0.f;

        glm::mat2 mat2() const {
            const float sin = glm::sin(rotation);
            const float cos = glm::cos(rotation);
            glm::mat2 rotMat{ {cos, sin}, {-sin, cos} };

            glm::mat2 scaleMat{ {scale.x, 0.f}, {0.f, scale.y} };

            return  rotMat * scaleMat;
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
        Transform2D mTransform{};

    private:
        VRE_GameObject(objectID id) : mID(id), mColor(1.f) {};

        objectID mID;
    };
}
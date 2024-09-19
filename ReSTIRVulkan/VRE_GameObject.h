#pragma once
#include <memory>
#include <unordered_map>
#include "VRE_Model.h"
#include "gtc/matrix_transform.hpp"
#include "VRE_ObjectComponents.h"

namespace VRE {
    class VRE_GameObject
    {
    public:
        using GameObjectID = unsigned int;
        using GameObjectsMap = std::unordered_map<GameObjectID, VRE_GameObject>;

        VRE_GameObject(GameObjectID id) : mID(id) , mColor(1.f) {}

        static VRE_GameObject CreateGameObject() {
            static GameObjectID currentID = 0;
            return VRE_GameObject(currentID++);
        }

        VRE_GameObject(const VRE_GameObject&) = delete;
        VRE_GameObject& operator=(const VRE_GameObject&) = delete;
        VRE_GameObject(VRE_GameObject&&) = default;
        VRE_GameObject& operator=(VRE_GameObject&&) = default;

        GameObjectID GetID() const { return mID; }

        std::shared_ptr<VRE_Model> mModel;
        glm::vec3 mColor;
        VRE::Transform mTransform{};

    private:
        GameObjectID mID;
    };
}
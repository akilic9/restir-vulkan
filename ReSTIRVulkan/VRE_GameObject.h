#pragma once
#include <memory>
#include <unordered_map>

#include "gtc/matrix_transform.hpp"
#include "VRE_glTFModel.h"
#include "VRE_ObjectComponents.h"
#include "VRE_SwapChain.h"

namespace VRE {

    struct GameObjectBufferData {
        glm::mat4 mModelMatrix{ 1.f };
        glm::mat4 mNormalMatrix{ 1.f };
    };

    class VRE_GameObjectManager;

    class VRE_GameObject
    {
    public:
        using GameObjectID = unsigned int;
        using GameObjectsMap = std::unordered_map<GameObjectID, VRE::VRE_GameObject>;

        VRE::Transform mTransform{};

        VRE_GameObject(VRE_GameObject&&) = default;
        VRE_GameObject(const VRE_GameObject&) = delete;
        VRE_GameObject& operator=(const VRE_GameObject&) = delete;
        VRE_GameObject& operator=(VRE_GameObject&&) = delete;

        GameObjectID GetID() const { return mID; }

        VkDescriptorBufferInfo GetBufferInfo();

        std::shared_ptr<VRE_glTFModel> mModel;

    private:
        VRE_GameObject(GameObjectID id, const VRE_GameObjectManager& manager);

        GameObjectID mID;
        const VRE_GameObjectManager& mManager;


        friend class VRE_GameObjectManager;
    };
}
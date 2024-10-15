#pragma once
#include <memory>
#include <unordered_map>

#include "VRE_glTFModel.h"
#include "gtc/matrix_transform.hpp"
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

        VRE_GameObject(VRE_GameObject&&) = default;
        VRE_GameObject(const VRE_GameObject&) = delete;
        VRE_GameObject& operator=(const VRE_GameObject&) = delete;
        VRE_GameObject& operator=(VRE_GameObject&&) = delete;

        GameObjectID GetID() const { return mID; }

        VkDescriptorBufferInfo GetBufferInfo(int frameIndex);

        std::shared_ptr<VRE_glTFModel> mModel;
        VRE::Transform mTransform{};

    private:
        VRE_GameObject(GameObjectID id, const VRE_GameObjectManager& manager);

        GameObjectID mID;
        const VRE_GameObjectManager& mManager;

        friend class VRE_GameObjectManager;
    };

    class VRE_GameObjectManager {
    public:
        static const int MAX_OBJECT_COUNT = 1000;

        VRE_GameObjectManager(VRE_Device& device);
        ~VRE_GameObjectManager();

        VRE_GameObjectManager(const VRE_GameObjectManager&) = delete;
        VRE_GameObjectManager& operator=(const VRE_GameObjectManager&) = delete;
        VRE_GameObjectManager(VRE_GameObjectManager&&) = delete;
        VRE_GameObjectManager& operator=(VRE_GameObjectManager&&) = delete;

        VRE_GameObject& CreateGameObject();

        VkDescriptorBufferInfo GetBufferInfoForGameObject(int frameIndex, VRE_GameObject::GameObjectID gObjectID) const;

        void UpdateBuffer(int frameIndex);

        VRE_GameObject::GameObjectsMap mGameObjectsMap{};
        std::vector<std::unique_ptr<VRE_Buffer>> mUboBuffers{ VRE_SwapChain::MAX_FRAMES_IN_FLIGHT };

    private:
        VRE_GameObject::GameObjectID mLastID = 0;
    };
}
#pragma once
#include "VRE_Device.h"
#include "VRE_GameObject.h"
#include "VRE_SharedContext.h"

namespace VRE {
    class VRE_GameObjectManager {
    public:
        VRE_GameObjectManager(VRE_SharedContext* sharedContext);
        ~VRE_GameObjectManager();

        VRE_GameObjectManager(const VRE_GameObjectManager&) = delete;
        VRE_GameObjectManager& operator=(const VRE_GameObjectManager&) = delete;
        VRE_GameObjectManager(VRE_GameObjectManager&&) = delete;
        VRE_GameObjectManager& operator=(VRE_GameObjectManager&&) = delete;

        VRE_GameObject& CreateGameObject();

        VkDescriptorBufferInfo GetBufferInfoForGameObject(VRE_GameObject::GameObjectID gObjectID) const;

        void Init();
        void Update(float deltaTime);

        inline VRE_GameObject::GameObjectsMap& GetGameObjectsMap() { return mGameObjectsMap; }

    private:
        VRE_GameObject::GameObjectID mLastID = 0;
        VRE_SharedContext* mSharedContext;
        VRE_GameObject::GameObjectsMap mGameObjectsMap{};
        std::vector<std::unique_ptr<VRE_Buffer>> mUboBuffers{ VRE_SwapChain::MAX_FRAMES_IN_FLIGHT };
    };
}
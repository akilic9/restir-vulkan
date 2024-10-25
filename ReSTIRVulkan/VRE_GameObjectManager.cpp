#include "VRE_GameObjectManager.h"

#include <numeric>

VRE::VRE_GameObjectManager::VRE_GameObjectManager(VRE_SharedContext* sharedContext)
    : mSharedContext(sharedContext) {}



VRE::VRE_GameObjectManager::~VRE_GameObjectManager() {}

VRE::VRE_GameObject& VRE::VRE_GameObjectManager::CreateGameObject()
{
    assert(mLastID < VRE::MAX_OBJECT_COUNT && "Max game object count exceeded!");
    VRE_GameObject object = VRE_GameObject{ mLastID++, *this };
    auto id = object.GetID();
    mGameObjectsMap.emplace(id, std::move(object));
    return mGameObjectsMap.at(id);
}

VkDescriptorBufferInfo VRE::VRE_GameObjectManager::GetBufferInfoForGameObject(VRE_GameObject::GameObjectID gObjectID) const
{
    return mUboBuffers[mSharedContext->mRenderer->GetFrameIndex()]->DescriptorInfoForIndex(gObjectID);
}

void VRE::VRE_GameObjectManager::Init()
{
    int alignment = std::lcm(mSharedContext->mDevice->mProperties.limits.nonCoherentAtomSize,
        mSharedContext->mDevice->mProperties.limits.minUniformBufferOffsetAlignment);

    for (int i = 0; i < mUboBuffers.size(); i++) {
        mUboBuffers[i] = std::make_unique<VRE_Buffer>(*mSharedContext->mDevice, sizeof(GameObjectBufferData), VRE::MAX_OBJECT_COUNT,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, alignment);

        mUboBuffers[i]->Map();
    }
}

void VRE::VRE_GameObjectManager::Update(float deltaTime)
{
    for (auto& e : mGameObjectsMap) {
        auto& obj = e.second;
        GameObjectBufferData data{};
        data.mModelMatrix = obj.mTransform.Mat4();
        data.mNormalMatrix = obj.mTransform.NormalMatrix();
        mUboBuffers[mSharedContext->mRenderer->GetFrameIndex()]->WriteToIndex(&data, e.first);
    }
    mUboBuffers[mSharedContext->mRenderer->GetFrameIndex()]->Flush();
}
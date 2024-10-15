#include "VRE_GameObject.h"

#include <numeric>

VRE::VRE_GameObjectManager::VRE_GameObjectManager(VRE_Device& device)
{
    int alignment = std::lcm(device.mProperties.limits.nonCoherentAtomSize, device.mProperties.limits.minUniformBufferOffsetAlignment);

    for (int i = 0; i < mUboBuffers.size(); i++) {
        mUboBuffers[i] = std::make_unique<VRE_Buffer>(device, sizeof(GameObjectBufferData), VRE_GameObjectManager::MAX_OBJECT_COUNT,
                                                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, alignment);

        mUboBuffers[i]->Map();
    }
}

VRE::VRE_GameObjectManager::~VRE_GameObjectManager()
{
}

VRE::VRE_GameObject& VRE::VRE_GameObjectManager::CreateGameObject()
{
    assert(mLastID < MAX_OBJECT_COUNT && "Max game object count exceeded!");
    VRE_GameObject object = VRE_GameObject{mLastID++, *this};
    auto id = object.GetID();
    mGameObjectsMap.emplace(id, std::move(object));
    return mGameObjectsMap.at(id);
}

VkDescriptorBufferInfo VRE::VRE_GameObjectManager::GetBufferInfoForGameObject(int frameIndex, VRE_GameObject::GameObjectID gObjectID) const
{
    return mUboBuffers[frameIndex]->DescriptorInfoForIndex(gObjectID);
}

void VRE::VRE_GameObjectManager::UpdateBuffer(int frameIndex)
{
    for (auto& e : mGameObjectsMap) {
        auto& obj = e.second;
        GameObjectBufferData data{};
        data.mModelMatrix = obj.mTransform.Mat4();
        data.mNormalMatrix = obj.mTransform.NormalMatrix();
        mUboBuffers[frameIndex]->WriteToIndex(&data, e.first);
    }
    mUboBuffers[frameIndex]->Flush();
}

VRE::VRE_GameObject::VRE_GameObject(GameObjectID id, const VRE_GameObjectManager& manager)
    : mID(id)
    , mManager(manager)
{}

VkDescriptorBufferInfo VRE::VRE_GameObject::GetBufferInfo(int frameIndex)
{
    return mManager.GetBufferInfoForGameObject(frameIndex, mID);
}
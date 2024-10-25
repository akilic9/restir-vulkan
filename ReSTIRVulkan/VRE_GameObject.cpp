#include "VRE_GameObject.h"
#include "VRE_GameObjectManager.h"

VRE::VRE_GameObject::VRE_GameObject(GameObjectID id, const VRE_GameObjectManager& manager)
    : mID(id)
    , mManager(manager)
{}

VkDescriptorBufferInfo VRE::VRE_GameObject::GetBufferInfo()
{
    return mManager.GetBufferInfoForGameObject(mID);
}
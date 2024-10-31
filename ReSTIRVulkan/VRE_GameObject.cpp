/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*/
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
#include "VRE_Descriptor.h"
#include <cassert>
#include <stdexcept>

VRE::VRE_DescriptorSetLayout::Builder& VRE::VRE_DescriptorSetLayout::Builder::AddBinding(uint32_t binding,
                                                                                         VkDescriptorType descriptorType,
                                                                                         VkShaderStageFlags stageFlags,
                                                                                         uint32_t count)
{
    assert(mBindings.count(binding) == 0 && "Binding already in use.");
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    mBindings[binding] = layoutBinding;
    return *this;
}

VRE::VRE_DescriptorPool::Builder& VRE::VRE_DescriptorPool::Builder::AddPoolSize(VkDescriptorType descriptorType, uint32_t count)
{
    mPoolSizeList.push_back({ descriptorType, count });
    return *this;
}

VRE::VRE_DescriptorPool::Builder& VRE::VRE_DescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags)
{
    mPoolFlags = flags;
    return *this;
}

VRE::VRE_DescriptorPool::Builder& VRE::VRE_DescriptorPool::Builder::SetMaxSets(uint32_t max)
{
    mMaxSets = max;
    return *this;
}

std::unique_ptr<VRE::VRE_DescriptorSetLayout> VRE::VRE_DescriptorSetLayout::Builder::Build() const
{
    return std::make_unique<VRE_DescriptorSetLayout>(mDevice, mBindings);
}

VRE::VRE_DescriptorSetLayout::VRE_DescriptorSetLayout(VRE_Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : mDevice(device)
    , mBindings(bindings)
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto e : mBindings)
        setLayoutBindings.push_back(e.second);

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(mDevice.GetVkDevice(), &descriptorSetLayoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor set layout!");
}

VRE::VRE_DescriptorSetLayout::~VRE_DescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(mDevice.GetVkDevice(), mDescriptorSetLayout, nullptr);
}

VRE::VRE_DescriptorPool::VRE_DescriptorPool(VRE_Device& device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes)
    : mDevice(device)
{
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(mDevice.GetVkDevice(), &descriptorPoolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool.");
}

VRE::VRE_DescriptorPool::~VRE_DescriptorPool()
{
    vkDestroyDescriptorPool(mDevice.GetVkDevice(), mDescriptorPool, nullptr);
}

std::unique_ptr<VRE::VRE_DescriptorPool> VRE::VRE_DescriptorPool::Builder::Build() const
{
    return std::make_unique<VRE_DescriptorPool>(mDevice, mMaxSets, mPoolFlags, mPoolSizeList);
}

bool VRE::VRE_DescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    // TODO: Might want to create a "DescriptorPoolManager" class that handles this case, and builds a new pool whenever an old pool fills up.
    if (vkAllocateDescriptorSets(mDevice.GetVkDevice(), &allocInfo, &descriptor) != VK_SUCCESS)
        return false;

    return true;
}

void VRE::VRE_DescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
{
    vkFreeDescriptorSets(mDevice.GetVkDevice(), mDescriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
}

void VRE::VRE_DescriptorPool::ResetPool()
{
    vkResetDescriptorPool(mDevice.GetVkDevice(), mDescriptorPool, 0);
}

VRE::VRE_DescriptorWriter::VRE_DescriptorWriter(VRE_DescriptorSetLayout& setLayout, VRE_DescriptorPool& pool)
    : mSetLayout(setLayout)
    , mPool(pool)
{}

VRE::VRE_DescriptorWriter& VRE::VRE_DescriptorWriter::WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
{
    assert(mSetLayout.mBindings.count(binding) == 1 && "Layout does not contain specified binding.");

    auto& bindingDescription = mSetLayout.mBindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding has single descriptor info, but binding expects multiple.");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    mWrites.push_back(write);
    return *this;
}

VRE::VRE_DescriptorWriter& VRE::VRE_DescriptorWriter::WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo)
{
    assert(mSetLayout.mBindings.count(binding) == 1 && "Layout does not contain specified binding.");

    auto& bindingDescription = mSetLayout.mBindings[binding];

    assert(bindingDescription.descriptorCount == 1 && "Binding has single descriptor info, but binding expects multiple.");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    mWrites.push_back(write);
    return *this;
}

bool VRE::VRE_DescriptorWriter::Build(VkDescriptorSet& set)
{
    if (!mPool.AllocateDescriptorSet(mSetLayout.GetDescriptorSetLayout(), set))
        return false;

    Overwrite(set);
    return true;
}

void VRE::VRE_DescriptorWriter::Overwrite(VkDescriptorSet& set)
{
    for (auto& write : mWrites)
        write.dstSet = set;

    vkUpdateDescriptorSets(mPool.mDevice.GetVkDevice(), mWrites.size(), mWrites.data(), 0, nullptr);
}

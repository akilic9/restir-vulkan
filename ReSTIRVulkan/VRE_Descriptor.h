/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*   Overvoorde, A. (2017). Khronos Vulkan Tutorial. [online] Vulkan.org. Available at: https://docs.vulkan.org/tutorial/latest/00_Introduction.html (Accessed 07 June 2024).
*/
#pragma once

#include "VRE_Device.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace VRE {
    class VRE_DescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(VRE_Device& device) : mDevice(device) {}

            Builder& AddBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
            std::unique_ptr<VRE_DescriptorSetLayout> Build() const;

        private:
            VRE_Device& mDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings{};
        };

        VRE_DescriptorSetLayout(VRE_Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~VRE_DescriptorSetLayout();

        VRE_DescriptorSetLayout(const VRE_DescriptorSetLayout&) = delete;
        VRE_DescriptorSetLayout& operator=(const VRE_DescriptorSetLayout&) = delete;

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return mDescriptorSetLayout; }

    private:
        VRE_Device& mDevice;
        VkDescriptorSetLayout mDescriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings;

        friend class VRE_DescriptorWriter;
    };

    class VRE_DescriptorPool {
    public:
        class Builder {
        public:
            Builder(VRE_Device& device) : mDevice(device) {}

            Builder& AddPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& SetMaxSets(uint32_t max);
            std::unique_ptr<VRE_DescriptorPool> Build() const;

        private:
            VRE_Device& mDevice;
            std::vector<VkDescriptorPoolSize> mPoolSizeList{};
            uint32_t mMaxSets = 1000;
            VkDescriptorPoolCreateFlags mPoolFlags = 0;
        };

        VRE_DescriptorPool(VRE_Device& device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VRE_DescriptorPool();
        VRE_DescriptorPool(const VRE_DescriptorPool&) = delete;
        VRE_DescriptorPool& operator=(const VRE_DescriptorPool&) = delete;

        bool AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void ResetPool();

    private:
        VRE_Device& mDevice;
        VkDescriptorPool mDescriptorPool;

        friend class VRE_DescriptorWriter;
    };

    class VRE_DescriptorWriter {
    public:
        VRE_DescriptorWriter(VRE_DescriptorSetLayout& setLayout, VRE_DescriptorPool& pool);

        VRE_DescriptorWriter& WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VRE_DescriptorWriter& WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool Build(VkDescriptorSet& set);
        void Overwrite(VkDescriptorSet& set);

    private:
        VRE_DescriptorSetLayout& mSetLayout;
        VRE_DescriptorPool& mPool;
        std::vector<VkWriteDescriptorSet> mWrites;
    };

}
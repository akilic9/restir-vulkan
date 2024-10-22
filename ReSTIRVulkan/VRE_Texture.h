#pragma once
#include <memory>
#include "VRE_Device.h"

namespace VRE {
    class VRE_Texture
    {
    public:
        struct SamplerProperties {
            VkFilter magFilter = VK_FILTER_LINEAR;
            VkFilter minFilter = VK_FILTER_LINEAR;
            VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        };

        VRE_Texture(VRE_Device& device, const std::string& filePath, SamplerProperties props);
        ~VRE_Texture();

        VRE_Texture(const VRE_Texture&) = delete;
        VRE_Texture& operator=(const VRE_Texture&) = delete;

        static std::unique_ptr<VRE_Texture> CreateTexture(VRE_Device& device, const std::string& filePath, SamplerProperties props = SamplerProperties{});

        VkDescriptorImageInfo GetImageInfo() const { return mDescriptor; }

    private:
        void CreateImage(const std::string& filePath);
        void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void CreateTextureImageView();
        void CreateTextureSampler(SamplerProperties props);
        void UpdateDescriptor();

        VRE_Device &mDevice;

        VkImage mTextureImage;
        VkDeviceMemory mTextureImageMemory;
        VkImageView mTextureImageView;
        VkSampler mTextureSampler;
        VkDescriptorImageInfo mDescriptor{};

        uint32_t mMipLevels;
    };
}
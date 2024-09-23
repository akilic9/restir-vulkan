#pragma once
#include <memory>
#include "VRE_Device.h"

namespace VRE {
    class VRE_Texture
    {
    public:
        VRE_Texture(VRE_Device& device, const std::string& filePath);
        ~VRE_Texture();

        VRE_Texture(const VRE_Texture&) = delete;
        VRE_Texture& operator=(const VRE_Texture&) = delete;

        static std::unique_ptr<VRE_Texture> CreateTexture(VRE_Device& device, const std::string& filePath);

        VkDescriptorImageInfo getImageInfo() const { return mDescriptor; }

    private:
        void CreateImage(const std::string& filePath);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void CreateTextureImageView();
        void CreateTextureSampler();
        void UpdateDescriptor();

        VRE_Device &mDevice;

        VkImage mTextureImage;
        VkDeviceMemory mTextureImageMemory;
        VkImageView mTextureImageView;
        VkSampler mTextureSampler;
        VkDescriptorImageInfo mDescriptor{};
    };
}
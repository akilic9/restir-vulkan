#pragma once
#include <memory>
#include "VRE_Device.h"

namespace VRE {
    class VRE_Texture
    {
    public:
        VRE_Texture(VRE_Device& device);
        ~VRE_Texture();

        VRE_Texture(const VRE_Texture&) = delete;
        VRE_Texture& operator=(const VRE_Texture&) = delete;

        void CreateTexture();
        void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void CreateTextureImageView();
        void CreateTextureSampler();

    private:
        VRE_Device &mDevice;

        VkImage mTextureImage;
        VkDeviceMemory mTextureImageMemory;
        VkImageView mTextureImageView;
        VkSampler mTextureSampler;
    };
}
/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*   Overvoorde, A. (2017). Khronos Vulkan Tutorial. [online] Vulkan.org. Available at: https://docs.vulkan.org/tutorial/latest/00_Introduction.html (Accessed 07 June 2024).
*   lukasino 1214 (2022). Textures - Vulkan / Game Engine tutorial[0]. [online] YouTube. Available at: https://www.youtube.com/watch?v=_AitmLEnP28 and https://github.com/lukasino1214/Game-Engine-Tutorial (Accessed 10 Aug. 2024).
*/
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

        VkDescriptorImageInfo GetDescImageInfo() const { return mDescriptor; }

    private:
        void CreateImage(const std::string& filePath);
        void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight);
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void CreateImageView();
        void CreateTextureSampler();
        void UpdateDescriptorInfo();

        VRE_Device &mDevice;

        VkImage mTextureImage;
        VkDeviceMemory mTextureImageMemory;
        VkImageView mImageView;
        VkSampler mTextureSampler;
        VkDescriptorImageInfo mDescriptor{};

        uint32_t mMipLevels;
    };
}
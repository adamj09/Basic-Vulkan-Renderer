#pragma once

#include "engine/device/device.hpp"

#include <memory>

namespace Renderer{
    class Sampler{
        public:
            struct SamplerConfig{
                VkFilter magFilter = VK_FILTER_LINEAR;
                VkFilter minFilter = VK_FILTER_LINEAR;
                VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                VkBool32 anisotropyEnable = VK_FALSE;
                float maxAnisotropy = 0.f;
                VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                VkBool32 unnormalizeCoordinates = VK_FALSE;
                VkBool32 compareEnable = VK_FALSE;
                VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
                VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                float mipLodBias = 0.f;
                float minLod = 0.f;
                float maxLod = 0.f;
            };

            Sampler(Device& device, SamplerConfig samplerConfig, unsigned int samplerId);
            ~Sampler();

            unsigned int getId() { return id; }
            VkSampler getSampler(){ return sampler; }
            static std::unique_ptr<Sampler> createSampler(Device& device, SamplerConfig samplerConfig);

        private:
            Device& device;
            VkSampler sampler;

            unsigned int id;
    };
}
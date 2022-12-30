#include "sampler.hpp"

#include <stdexcept>

namespace Renderer{
    Sampler::Sampler(Device& device, SamplerConfig samplerConfig, unsigned int samplerId) : device{device}, id{samplerId} {
        VkSamplerCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.magFilter = samplerConfig.magFilter;
        createInfo.minFilter = samplerConfig.minFilter;
        createInfo.addressModeU = samplerConfig.addressModeU;
        createInfo.addressModeV = samplerConfig.addressModeV;
        createInfo.addressModeW = samplerConfig.addressModeW;
        createInfo.anisotropyEnable = samplerConfig.anisotropyEnable;
        createInfo.maxAnisotropy = samplerConfig.maxAnisotropy;
        createInfo.borderColor = samplerConfig.borderColor;
        createInfo.unnormalizedCoordinates = samplerConfig.unnormalizeCoordinates;
        createInfo.compareEnable = samplerConfig.compareEnable;
        createInfo.compareOp = samplerConfig.compareOp;
        createInfo.mipmapMode = samplerConfig.mipmapMode;
        createInfo.mipLodBias = samplerConfig.mipLodBias;
        createInfo.minLod = samplerConfig.minLod;
        createInfo.maxLod = samplerConfig.maxLod;

        if(vkCreateSampler(device.getDevice(), &createInfo, nullptr, &sampler) != VK_SUCCESS)
            throw std::runtime_error("Failed to create sampler.");
    }

    Sampler::~Sampler(){
        vkDestroySampler(device.getDevice(), sampler, nullptr);
    }

    std::unique_ptr<Sampler> Sampler::createSampler(Device& device, SamplerConfig samplerConfig){
        static unsigned int currentId = 0;
        return std::make_unique<Sampler>(device, samplerConfig, currentId++);
    }
}
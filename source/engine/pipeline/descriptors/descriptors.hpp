#pragma once

#include "engine/device/device.hpp"
#include "engine/buffer/buffer.hpp"

#include <vector>
#include <unordered_map>
#include <cassert>

namespace Renderer{
    class DescriptorSetLayout{
        public:
            DescriptorSetLayout(Device& device);
            ~DescriptorSetLayout();

            VkDescriptorSetLayout getLayout() { return layout; }
            void addBinding(uint32_t descriptorCount, VkDescriptorType type, VkShaderStageFlags stageFlags, VkSampler* pImmutableSamplers = nullptr);
            void buildLayout(VkDescriptorSetLayoutCreateFlags flags = 0);

            VkWriteDescriptorSet writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
            VkWriteDescriptorSet writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        private:
            Device& device;
            VkDescriptorSetLayout layout;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
            std::vector<VkWriteDescriptorSet> writes;
        friend class DescriptorPool;
    };

    class DescriptorPool{
        public:
            DescriptorPool(Device& device);
            ~DescriptorPool();

            std::unordered_map<uint32_t, VkDescriptorSet> getSets() { return allocatedSets; }

            void addPoolSize(VkDescriptorType type, uint32_t count);
            void buildPool(uint32_t maxSets, VkDescriptorPoolCreateFlags flags = 0);
            void allocateSet(VkDescriptorSetLayout descriptorSetLayout);
            void updateSet(uint32_t setIndex, std::vector<VkWriteDescriptorSet> writes);
            void addNewSets(VkDescriptorSetLayout descriptorSetLayout, std::vector<VkWriteDescriptorSet> writes, uint32_t count);

        private:
            Device& device;
            VkDescriptorPool descriptorPool;
            std::vector<VkDescriptorPoolSize> poolSizes;
            std::unordered_map<uint32_t, VkDescriptorSet> allocatedSets;
    };
}
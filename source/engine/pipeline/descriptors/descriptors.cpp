#include "descriptors.hpp"

#include <stdexcept>
#include <iostream>

namespace Renderer{
    DescriptorPool::DescriptorPool(Device& device) : device{device}{}

    DescriptorPool::~DescriptorPool(){
        vkDestroyDescriptorPool(device.getDevice(), descriptorPool, nullptr);
    }

    void DescriptorPool::addPoolSize(VkDescriptorType type, uint32_t count){
        VkDescriptorPoolSize newPoolSize = {};
        newPoolSize.type = type;
        newPoolSize.descriptorCount = count;
        poolSizes.push_back(newPoolSize);
    }

    void DescriptorPool::buildPool(uint32_t maxSets, VkDescriptorPoolCreateFlags flags){
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;
        poolInfo.flags = flags;

        if(vkCreateDescriptorPool(device.getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create descriptor pool.");
    }

    void DescriptorPool::allocateSet(VkDescriptorSetLayout descriptorSetLayout){
        VkDescriptorSet set;
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        if(vkAllocateDescriptorSets(device.getDevice(), &allocInfo, &set) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate descriptor set.");
        allocatedSets.push_back(set);
    }

    void DescriptorPool::updateSet(uint32_t setIndex, std::vector<VkWriteDescriptorSet> writes){
        for(size_t i = 0; i < writes.size(); i++)
            writes[i].dstSet = allocatedSets[setIndex];
        vkUpdateDescriptorSets(device.getDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

    DescriptorSetLayout::DescriptorSetLayout(Device& device) : device{device}{}
    DescriptorSetLayout::~DescriptorSetLayout(){}

    void DescriptorSetLayout::addBinding(uint32_t descriptorCount, VkDescriptorType type, VkShaderStageFlags stageFlags, VkSampler* pImmutableSamplers){
        uint32_t binding = bindings.size();
        VkDescriptorSetLayoutBinding newBinding {};
        newBinding.binding = binding;
        newBinding.descriptorCount = descriptorCount;
        newBinding.descriptorType = type;
        newBinding.stageFlags = stageFlags;
        newBinding.pImmutableSamplers = pImmutableSamplers;
        bindings.emplace(binding, newBinding);
    }

    void DescriptorSetLayout::buildLayout(VkDescriptorSetLayoutCreateFlags flags){
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto binding : bindings) 
            setLayoutBindings.push_back(binding.second);
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        layoutInfo.pBindings = setLayoutBindings.data();
        layoutInfo.flags = flags;

        if(vkCreateDescriptorSetLayout(device.getDevice(), &layoutInfo, nullptr, &layout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create descriptor set layout.");
    }

    VkWriteDescriptorSet DescriptorSetLayout::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo){
        auto &bindingDescription = bindings[binding];
        VkWriteDescriptorSet newWrite{};
        newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        newWrite.dstBinding = binding;
        newWrite.dstArrayElement = 0;
        newWrite.descriptorType = bindingDescription.descriptorType;
        newWrite.descriptorCount = bindingDescription.descriptorCount;
        newWrite.pBufferInfo = bufferInfo;
        return newWrite;
    }

    VkWriteDescriptorSet DescriptorSetLayout::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo){
        auto &bindingDescription = bindings[binding];
        VkWriteDescriptorSet newWrite{};
        newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        newWrite.dstBinding = binding;
        newWrite.dstArrayElement = 0;
        newWrite.descriptorType = bindingDescription.descriptorType;
        newWrite.descriptorCount = bindingDescription.descriptorCount;
        newWrite.pImageInfo = imageInfo;
        return newWrite;
    }
}
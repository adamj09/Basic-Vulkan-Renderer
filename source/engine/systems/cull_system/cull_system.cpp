#include "cull_system.hpp"

#include "engine/swap_chain/swap_chain.hpp"

#include <stdexcept>

namespace Renderer{

    CullSystem::CullSystem(Device& device) 
    : device{device} {

    }

    CullSystem::~CullSystem(){
        vkDestroyDescriptorSetLayout(device.getDevice(), cullSetLayout->getLayout(), nullptr);
        vkDestroyPipelineLayout(device.getDevice(), cullPipelineLayout, nullptr);
    }

    void CullSystem::cullScene(VkCommandBuffer commandBuffer, uint32_t frameIndex, uint32_t indirectCommandCount){
        cullPipeline->bind(commandBuffer);

        uint32_t setIndex = (frameIndex + SwapChain::MAX_FRAMES_IN_FLIGHT) * 1; // Multiply by number of groups of uniform sets that come beforehand

        vkCmdDispatch(commandBuffer, indirectCommandCount / 64, 1, 1);
    }

    void CullSystem::setupDescriptors(){
        std::vector<VkWriteDescriptorSet> cullLayoutWrites; /*{
            cullSetLayout->writeBuffer(0, &sceneUniformBufferInfo),
            cullSetLayout->writeBuffer(1, &objectBufferInfo),
            cullSetLayout->writeBuffer(2, &indirectCommandsBufferInfo),
        };*/

        // Compute cull layout setup
        cullSetLayout = std::make_unique<DescriptorSetLayout>(device);
        cullSetLayout->addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0);            // Uniform scene info
        cullSetLayout->addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT, 1);    // Object info
        cullSetLayout->addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2);            // Indexed indirect draw data
        cullSetLayout->buildLayout();

        cullDescriptorPool->addNewSets(cullSetLayout->getLayout(), cullLayoutWrites, 2);
    }

    void CullSystem::createComputePipelineLayout(){
        auto layout = cullSetLayout->getLayout();
        VkPipelineLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &layout;
        layoutInfo.pushConstantRangeCount = 0;
        layoutInfo.pPushConstantRanges = nullptr;

        if(vkCreatePipelineLayout(device.getDevice(), &layoutInfo, nullptr, &cullPipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create compute pipeline layout.");
    }

    void CullSystem::createComputePipeline(){
        assert(cullPipelineLayout != nullptr && "Cannot create compute pipeline before compute pipeline layout.");
        cullPipeline = std::make_unique<ComputePipeline>(
            device,
            "../source/spirv_shaders/instance_cull.comp.spv",
            cullPipelineLayout
        );
    }
}
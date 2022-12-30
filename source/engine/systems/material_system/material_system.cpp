#include "material_system.hpp"

#include <stdexcept>
#include <cassert>

namespace Renderer{
    MaterialSystem::MaterialSystem(Device& device, VkRenderPass renderPass) : device{device}, renderPass{renderPass}{
        createPipelineLayout();
        createPipeline();
    }

    MaterialSystem::~MaterialSystem(){
        vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
    }

    void MaterialSystem::createPipelineLayout(){
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 0;
        layoutInfo.pSetLayouts = nullptr;
        layoutInfo.pushConstantRangeCount = 0;
        layoutInfo.pPushConstantRanges = nullptr;

        if(vkCreatePipelineLayout(device.getDevice(), &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout.");
    }

    void MaterialSystem::createPipeline(){
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout.");

        GraphicsPipelineConfigInfo configInfo{};
        configInfo.pipelineLayout = pipelineLayout;
        configInfo.renderPass = renderPass;

        pipeline = std::make_unique<GraphicsPipeline>(
            device,
            "C:/Programming/C++_Projects/renderer/source/shaders/material.vert",
            "C:/Programming/C++_Projects/renderer/source/shaders/material.frag",
            configInfo
        );
    }

    void MaterialSystem::addMaterial(Material newMaterial){
        materials.emplace(1, newMaterial);
    }
}
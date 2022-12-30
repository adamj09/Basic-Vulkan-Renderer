#pragma once

#include "engine/device/device.hpp"
#include "engine/pipeline/pipeline.hpp"
#include "engine/material/material.hpp"

#include <unordered_map>

namespace Renderer{
    class MaterialSystem{
        public:
            MaterialSystem(Device& device, VkRenderPass renderPass);
            ~MaterialSystem();

            void addMaterial(Material newMaterial); // Add a material to the vector for rendering

        private:
            void createPipelineLayout();
            void createPipeline();

            Device& device;
            VkRenderPass renderPass;

            VkPipelineLayout pipelineLayout;
            std::unique_ptr<GraphicsPipeline> pipeline;

            Material::Map materials;
    };
}
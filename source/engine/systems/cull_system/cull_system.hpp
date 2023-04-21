#pragma once

#include "engine/device/device.hpp"
#include "engine/pipeline/pipeline.hpp"
#include "engine/pipeline/descriptors/descriptors.hpp"

#include <memory>

namespace Renderer{
    class CullSystem{
        public:
            CullSystem(Device& device);
            ~CullSystem();

            void cullScene(VkCommandBuffer commandBuffer, uint32_t frameIndex, uint32_t indirectCommandCount);

        private:
            void setupDescriptors();
            void createComputePipelineLayout();
            void createComputePipeline();

            Device& device;

            std::unique_ptr<DescriptorPool> cullDescriptorPool;
            std::unique_ptr<DescriptorSetLayout> cullSetLayout;

            std::unique_ptr<ComputePipeline> cullPipeline;
            VkPipelineLayout cullPipelineLayout;
    };
}
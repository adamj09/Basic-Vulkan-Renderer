#pragma once
#include "engine/device/device.hpp"

#include "engine/pipeline/pipeline.hpp"
#include "engine/pipeline/descriptors/descriptors.hpp"
#include "engine/swap_chain/swap_chain.hpp"
#include "engine/buffer/buffer.hpp"
#include "engine/camera/camera.hpp"
#include "engine/scene/scene.hpp"

#include <memory>

namespace Renderer{
    class RenderSystem{
        public:
            struct InstanceData{
                glm::mat4 modelMatrix{1.f};
                glm::mat4 normalMatrix{1.f};

                glm::vec3 translation;
                glm::vec3 scale;
                glm::vec3 rotation;

                unsigned int materialId;
                unsigned int modelId;
            };

            struct UniformData{
                glm::mat4 projection{1.f};
                glm::mat4 view{1.f};
                glm::mat4 inverseView{1.f};

                bool enableFrustumCulling;

                glm::vec4 frustumPlanes[6];
                glm::vec4 frustumCorners[8];
                uint32_t shapesToCull;
            } uniformData;

            RenderSystem(Device& device, VkRenderPass renderPass);
            ~RenderSystem();

            void updateUniformBuffer(Camera camera, uint32_t frameIndex);
            void drawScene(VkCommandBuffer commandBuffer, uint32_t frameIndex);

        private:
            void initializeRenderSystem();

            void setupScene();
            void setupDescriptorSets();

            void createGraphicsPipelineLayout();
            void createGraphicsPipeline();

            void createComputePipelineLayout();
            void createComputePipeline();

            void createIndirectCommands();
            void setupInstanceData();
            
            size_t padUniformBufferSize(size_t originalSize);
            uint32_t maxMiplevels();

            Device& device;
            VkRenderPass renderPass;

            Scene scene;

            std::unique_ptr<GraphicsPipeline> renderPipeline;
            VkPipelineLayout renderPipelineLayout;

            std::unique_ptr<ComputePipeline> cullPipeline;
            VkPipelineLayout cullPipelineLayout;

            std::vector<std::unique_ptr<Buffer>> instanceBuffers;
            
            std::vector<InstanceData> instanceData;

            std::unique_ptr<Buffer> indirectCommandsBuffer;
            std::vector<VkDrawIndexedIndirectCommand> indirectCommands;

            std::unique_ptr<DescriptorPool> globalPool;
            std::unique_ptr<DescriptorSetLayout> globalSetLayout;

            std::vector<std::unique_ptr<Buffer>> uniformBuffers;
            uint32_t latestBinding = 0;

            uint32_t instanceCount;
            uint32_t meshCount;
    };
}
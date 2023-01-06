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
            struct ModelMatrixData{            
                glm::mat4 modelMatrix{1.f};
                glm::mat4 normalMatrix{1.f};
            };

            struct InstanceCullData{            // This struct is to be used on a per object/instance basis
                unsigned int instanceIndex;     // Instance index (also objectId)
                unsigned int indirectCommandID; // IndirectCommandId (also modelId associated to this object)
            };

            struct CullData{                    // Cull data, uniform across all instances
                bool enableOcclusionCulling;
                bool enableFrustumCulling;

                glm::vec4 frustumPlanes[6];
                glm::vec4 frustumCorners[8];
            };

            struct UniformData{                 // Camera data
                glm::mat4 projection{1.f};
                glm::mat4 view{1.f};
                glm::mat4 inverseView{1.f};
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

            std::unique_ptr<ComputePipeline> cullPipeline;
            VkPipelineLayout cullPipelineLayout;

            std::unique_ptr<GraphicsPipeline> renderPipeline;
            VkPipelineLayout renderPipelineLayout;

            std::vector<std::unique_ptr<Buffer>> instanceCullBuffers;
            std::vector<InstanceCullData> instanceCullData;

            std::unique_ptr<Buffer> indirectCommandsBuffer;
            std::vector<VkDrawIndexedIndirectCommand> indirectCommands;

            std::unique_ptr<DescriptorPool> globalPool;

            std::unique_ptr<DescriptorSetLayout> cullSetLayout;
            std::unique_ptr<DescriptorSetLayout> renderSetLayout;

            std::vector<std::unique_ptr<Buffer>> uniformBuffers;
            uint32_t latestBinding = 0;

            uint32_t totalInstanceCount = 0;
    };
}
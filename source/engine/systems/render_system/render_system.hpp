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
            struct ModelMatrixInfo{            
                glm::mat4 modelMatrix{1.f};
                glm::mat4 normalMatrix{1.f};
            };

            struct SceneUniform{  
                glm::mat4 projection{1.f};
                glm::mat4 view{1.f};
                glm::mat4 inverseView{1.f};

                BoundingBox viewBoundingBox;

                bool enableOcclusionCulling = true;
                bool enableFrustumCulling = true;

                uint32_t instanceCount = 0;
            } sceneUniform;

            struct ObjectInfo{
                unsigned int modelId;
                unsigned int diffuseId;

                unsigned int boundingSphereRadius;
                glm::vec3 boundingSphereCenter;
            } objectInfo;

            RenderSystem(Device& device, VkRenderPass renderPass);
            ~RenderSystem();

            void updateSceneUniform(Camera camera, uint32_t frameIndex);
            void cullScene(VkCommandBuffer commandBuffer, uint32_t frameIndex);
            void drawScene(VkCommandBuffer commandBuffer, uint32_t frameIndex);

        private:
            void setupScene();

            void createDrawIndirectCommands();
            void createUniformBuffers();

            void createVertexBuffers();
            void createIndexBuffer();

            void setupDescriptorSets();

            void createGraphicsPipelineLayout();
            void createGraphicsPipeline();

            void createComputePipelineLayout();
            void createComputePipeline();
            
            size_t padUniformBufferSize(size_t originalSize);

            Device& device;
            VkRenderPass renderPass;

            Scene scene;

            std::unique_ptr<ComputePipeline> cullPipeline;
            VkPipelineLayout cullPipelineLayout;

            std::unique_ptr<GraphicsPipeline> renderPipeline;
            VkPipelineLayout renderPipelineLayout;

            std::vector<std::unique_ptr<Buffer>> sceneUniformBuffers;

            std::vector<std::unique_ptr<Buffer>> objectInfoBuffers;
            uint32_t objectInfoDynamicAlignment;

            std::vector<std::unique_ptr<Buffer>> indirectCommandsBuffers;
            std::vector<VkDrawIndexedIndirectCommand> indirectCommands;

            std::unique_ptr<DescriptorPool> globalPool;

            std::unique_ptr<DescriptorSetLayout> cullSetLayout;
            std::unique_ptr<DescriptorSetLayout> renderSetLayout;

            std::unique_ptr<Buffer> vertexBuffer;
            std::vector<Model::Vertex> vertices{};
            uint32_t totalVertexCount = 0;

            std::unique_ptr<Buffer> indexBuffer;
            std::vector<uint32_t> indices{};
            uint32_t totalIndexCount = 0;

            std::unique_ptr<Buffer> instanceBuffer;

            uint32_t totalInstanceCount = 0;
    };
}
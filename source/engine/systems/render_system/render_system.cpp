#include "render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <cassert>
#include <algorithm>

namespace Renderer{
    RenderSystem::RenderSystem(Device& device, VkRenderPass renderPass) 
    : device{device}, renderPass{renderPass}{
        initializeRenderSystem();
    }

    RenderSystem::~RenderSystem(){
        vkDestroyDescriptorSetLayout(device.getDevice(), cullSetLayout->getLayout(), nullptr);
        vkDestroyPipelineLayout(device.getDevice(), cullPipelineLayout, nullptr);

        vkDestroyDescriptorSetLayout(device.getDevice(), renderSetLayout->getLayout(), nullptr);
        vkDestroyPipelineLayout(device.getDevice(), renderPipelineLayout, nullptr);
    }

    void RenderSystem::initializeRenderSystem(){
        setupScene();

        createIndirectDrawCommands();
        setupInstanceData();

        setupDescriptorSets();

        createComputePipelineLayout();
        createComputePipeline();

        createGraphicsPipelineLayout();
        createGraphicsPipeline();
    }

    void RenderSystem::setupScene(){
        // All of the below is temporary scene setup for testing, these actions should rather be done in a menu by the user.
        // Diffuse texture sampler
        Sampler::SamplerConfig textureSamplerConfig{};
        textureSamplerConfig.anisotropyEnable = VK_TRUE;
        textureSamplerConfig.maxAnisotropy = 16.f;
        textureSamplerConfig.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        textureSamplerConfig.maxLod = 100.f;
        scene.createSampler(device, textureSamplerConfig);
        
        // Load assets
        scene.loadTexturesWithSampler(device, 0);
        scene.loadModels(device);

        // spongebob object
        scene.createObject();
        scene.objects.at(0).modelId = 0; // spongebob model
        scene.objects.at(0).textureId = 0; // spongebob texture
        scene.objects.at(0).transform.translation = {1.5f, .5f, 0.f};
        scene.objects.at(0).transform.rotation = {glm::radians(180.f), 0.f, 0.f};

        // sample object
        scene.createObject();
        scene.objects.at(1).modelId = 1; // sample model
        scene.objects.at(1).textureId = 1; // sampler texture
        scene.objects.at(1).transform.translation = {-.5f, .5f, 0.f};
        scene.objects.at(1).transform.scale = {4.f, 4.f, 4.f};
    }

    void RenderSystem::createIndirectDrawCommands(){
        indirectCommands.clear();
        totalInstanceCount = 0;
        // TODO: sort through models that don't have indices and create commands for them and draw them seperately.
        // TODO: glTF models may have multiple nodes with different meshes; may need to have multiple commands per object.
        for(size_t i = 0; i < scene.models.size(); i++){
            uint32_t instanceCount = 0;
            // Get the number of objects that use this model, this will become the number of instances of this model.
            for(size_t j = 0; j < scene.objects.size(); j++)
                if(scene.objects.at(j).modelId == scene.models.at(i)->getId())
                    instanceCount++;
                    
            // Create a new indexedIndirectCommand for each unique model
            VkDrawIndexedIndirectCommand newIndexedIndirectCommand;
            newIndexedIndirectCommand.firstIndex = 0; // Currently there's one mesh per object so this will always be 0.
            newIndexedIndirectCommand.instanceCount = instanceCount; // Number of objects that use this unique model
            newIndexedIndirectCommand.firstInstance = 0; // Should always be 0 at the moment (start draw command at first instance of this model)
            newIndexedIndirectCommand.vertexOffset = 0; // Should always be 0
            newIndexedIndirectCommand.indexCount = scene.models.at(scene.objects.at(i).modelId)->getIndexCount(); // Number of indices the unique model has
            indirectCommands.push_back(newIndexedIndirectCommand); // Add the new command to the vector

            totalInstanceCount += instanceCount; // Add to the total instance count to the current number of instances
        }

        // Send indirect commands to GPU memory (2 buffers are need for double-buffering)
        indirectCommandsBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++){
            Buffer stagingBuffer{
                device,
                1,
                indirectCommands.size() * sizeof(VkDrawIndexedIndirectCommand),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            };

            stagingBuffer.map();
            stagingBuffer.writeToBuffer(indirectCommands.data());

            indirectCommandsBuffers[i] = std::make_unique<Buffer>(
                device,
                1,
                indirectCommands.size() * sizeof(VkDrawIndexedIndirectCommand),
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );
            
            stagingBuffer.copyBuffer(indirectCommandsBuffers[i]->getBuffer(), indirectCommandsBuffers[i]->getSize());
        }
    }

    void RenderSystem::setupInstanceData(){
        instanceCullInfos.resize(totalInstanceCount);
        // Info set once as for all objects as the default, this info can be updated in the updateScene() function.
        for(uint32_t i = 0; i < totalInstanceCount; i++){
            auto object = scene.objects.at(i);
            instanceCullInfos[i].instanceIndex = object.getId(); // since we have one instance per object, instance index == objectId
            instanceCullInfos[i].indirectCommandID = object.modelId; // since we are creating one indirect command per model, indirect command id = object.modelId
        }
        // Two instance data buffers needed with duplicate data since we're double buffering
        instanceCullBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++){
            Buffer stagingBuffer{
                device,
                1,
                instanceCullInfos.size() * sizeof(InstanceCullInfo),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            };

            stagingBuffer.map();
            stagingBuffer.writeToBuffer(instanceCullInfos.data());

            instanceCullBuffers[i] = std::make_unique<Buffer>(
                device,
                1,
                instanceCullInfos.size() * sizeof(InstanceCullInfo),
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );

            stagingBuffer.copyBuffer(instanceCullBuffers[i]->getBuffer(), instanceCullBuffers[i]->getSize());
        }
    }

    void RenderSystem::setupDescriptorSets(){
        // Universal Matrix Data
        uniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uniformBuffers.size(); i++) {
            uniformBuffers[i] = std::make_unique<Buffer>(device, 1, sizeof(UniformInfo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uniformBuffers[i]->map();
        } 
        
        // Pool Setup
        globalPool = std::make_unique<DescriptorPool>(device);
        globalPool->addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT);    // Uniform data (for rendrer pipeline) 
        globalPool->addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT);    // Indirect draw buffers (for gpu-created draw commands)
        globalPool->buildPool(SwapChain::MAX_FRAMES_IN_FLIGHT * 2); // Multiply max frames in flight by number of sets

        // Layout Setup (Bindings are set in order of when they are added)
        // Render set layout (vert and frag shaders)
        renderSetLayout = std::make_unique<DescriptorSetLayout>(device);
        renderSetLayout->addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS);    // binding 0 (Uniform data)
        renderSetLayout->buildLayout();

        cullSetLayout = std::make_unique<DescriptorSetLayout>(device);
        cullSetLayout->addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT);    // binding 0 (Indirect draw data)
        cullSetLayout->addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT);    // binding 1 (Global cull data)
        cullSetLayout->buildLayout();

        // Render Set
        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++){
            uint32_t setIndex = i * SwapChain::MAX_FRAMES_IN_FLIGHT;
            VkDescriptorBufferInfo indirectBufferInfo = indirectCommandsBuffers[i]->descriptorInfo();
            VkDescriptorBufferInfo uniformBufferInfo = uniformBuffers[i]->descriptorInfo();

            std::vector<VkWriteDescriptorSet> renderLayoutWrites {
                renderSetLayout->writeBuffer(0, &uniformBufferInfo)
            };
            std::vector<VkWriteDescriptorSet> cullLayoutWrites {
                cullSetLayout->writeBuffer(0, &indirectBufferInfo),
                cullSetLayout->writeBuffer(1, &uniformBufferInfo)
            };

            globalPool->allocateSet(renderSetLayout->getLayout());
            globalPool->updateSet(setIndex, renderLayoutWrites);

            globalPool->allocateSet(cullSetLayout->getLayout());
            globalPool->updateSet(setIndex + 1, cullLayoutWrites);
        }
    }

    void RenderSystem::createComputePipelineLayout(){
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

    void RenderSystem::createComputePipeline(){
        assert(cullPipelineLayout != nullptr && "Cannot create compute pipeline before compute pipeline layout.");
        cullPipeline = std::make_unique<ComputePipeline>(
            device,
            "../source/spirv_shaders/indirect_cull.comp.spv",
            cullPipelineLayout
        );
    }

    void RenderSystem::createGraphicsPipelineLayout(){
        auto layout = renderSetLayout->getLayout();
        VkPipelineLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &layout;
        layoutInfo.pushConstantRangeCount = 0;
        layoutInfo.pPushConstantRanges = nullptr;

        if(vkCreatePipelineLayout(device.getDevice(), &layoutInfo, nullptr, &renderPipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics pipeline layout.");
    }

    void RenderSystem::createGraphicsPipeline(){
        assert(renderPipelineLayout != nullptr && "Cannot create graphics pipeline before graphics pipeline layout.");
        GraphicsPipelineConfigInfo configInfo = {};
        GraphicsPipeline::defaultPipelineConfigInfo(configInfo);
        configInfo.pipelineLayout = renderPipelineLayout;
        configInfo.renderPass = renderPass;
        renderPipeline = std::make_unique<GraphicsPipeline>(
            device,
            "../source/spirv_shaders/main.vert.spv",
            "../source/spirv_shaders/main.frag.spv",
            configInfo
        );
    }

    void RenderSystem::updateCameraData(Camera camera, uint32_t frameIndex){
        // TODO: add check to see if camera view changed so needless updates are not performed
        uniformData.projection = camera.getProjection();
        uniformData.view = camera.getView();
        uniformData.inverseView = camera.getInverseView();

        uniformData.frustumPlanes[0] = glm::vec4();
        uniformData.frustumPlanes[1] = glm::vec4();
        uniformData.frustumPlanes[2] = glm::vec4();
        uniformData.frustumPlanes[3] = glm::vec4();
        uniformData.frustumPlanes[4] = glm::vec4();
        uniformData.frustumPlanes[5] = glm::vec4();
        uniformData.frustumPlanes[6] = glm::vec4();
        
        uniformData.enableFrustumCulling = true;
        uniformData.enableOcclusionCulling = true;

        uniformData.instanceCount = totalInstanceCount;

        uniformBuffers[frameIndex]->writeToBuffer(&uniformData);
        uniformBuffers[frameIndex]->flush();
    }

    void RenderSystem::drawScene(VkCommandBuffer commandBuffer, uint32_t frameIndex){
        cullPipeline->bind(commandBuffer);
        renderPipeline->bind(commandBuffer);
        vkCmdDrawIndexedIndirect(commandBuffer, indirectCommandsBuffers[frameIndex]->getBuffer(), 0, static_cast<uint32_t>(indirectCommands.size()), sizeof(VkDrawIndexedIndirectCommand));
    }

    size_t RenderSystem::padUniformBufferSize(size_t originalSize){
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);

        size_t minUboAlignment = properties.limits.minUniformBufferOffsetAlignment;
	    size_t alignedSize = originalSize;
	    if (minUboAlignment > 0) 
            alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	    return alignedSize;
    }

    /*uint32_t RenderSystem::maxMiplevels(){
        uint32_t greatestMipLevelCount = 0;
        std::vector<uint32_t> mipLevels;
        for(int i = 0; i < objects.size(); i++){
            auto obj = objects.at(i);
            if(obj.textureImage == nullptr) continue;
            mipLevels.push_back(obj.textureImage->getMipLevels());
        }
        for(int i = 0; i < mipLevels.size(); i++)
            if(mipLevels[i] > greatestMipLevelCount)
                greatestMipLevelCount = mipLevels[i];

        return greatestMipLevelCount;
    }*/
}
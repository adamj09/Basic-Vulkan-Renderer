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
        setupDescriptorSets();

        createComputePipelineLayout();
        createComputePipeline();

        createGraphicsPipelineLayout();
        createGraphicsPipeline();

        createIndirectCommands();
        setupInstanceData();
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

    void RenderSystem::setupDescriptorSets(){
        // Universal Matrix Data
        uniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uniformBuffers.size(); i++) {
            uniformBuffers[i] = std::make_unique<Buffer>(device, 1, sizeof(UniformData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uniformBuffers[i]->map();
        } 
        
        // Pool Setup
        globalPool = std::make_unique<DescriptorPool>(device);
        globalPool->addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT);    // Uniform data (for rendrer pipeline)
        globalPool->addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT);    // Instance data (for cull pipeline) 
        globalPool->buildPool(SwapChain::MAX_FRAMES_IN_FLIGHT);

        // Layout Setup (Bindings are set in order of when they are added)
        // Render set layout (vert and frag shaders)
        renderSetLayout = std::make_unique<DescriptorSetLayout>(device);
        renderSetLayout->addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS);    // binding 0 (Uniform data)
        renderSetLayout->buildLayout();
        // Cull set layout (compute shader)
        cullSetLayout = std::make_unique<DescriptorSetLayout>(device);
        cullSetLayout->addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT);       // binding 0 (Instance data)
        cullSetLayout->buildLayout();

        // Render Set
        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++){
            VkDescriptorBufferInfo uniformBufferInfo = uniformBuffers[i]->descriptorInfo();

            std::vector<VkWriteDescriptorSet> newWrites{
                renderSetLayout->writeBuffer(0, &uniformBufferInfo)
            };

            globalPool->allocateSet(renderSetLayout->getLayout());
            globalPool->updateSet(i, newWrites);
        }

        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++){
            // Fill instance buffer info
            VkDescriptorBufferInfo instanceCullBufferInfo = instanceCullBuffers[i]->descriptorInfo();
            std::vector<VkWriteDescriptorSet> newWrites {
                cullSetLayout->writeBuffer(0, &instanceCullBufferInfo)
            };

            globalPool->allocateSet(renderSetLayout->getLayout());
            // add SwapChain::MAX_FRAMES_IN_FLIGHT to index to offset update index by the # of sets in the previous layout
            globalPool->updateSet(i + SwapChain::MAX_FRAMES_IN_FLIGHT, newWrites);
        }
    }

    void RenderSystem::createGraphicsPipelineLayout(){
        auto layout = renderSetLayout->getLayout();
        VkPipelineLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 0;
        layoutInfo.pSetLayouts = nullptr;
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
            "C:/Programming/C++_Projects/renderer/source/spirv_shaders/main.vert.spv",
            "C:/Programming/C++_Projects/renderer/source/spirv_shaders/main.frag.spv",
            configInfo
        );
    }

    void RenderSystem::createComputePipelineLayout(){
        VkPipelineLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 0;
        layoutInfo.pSetLayouts = nullptr;
        layoutInfo.pushConstantRangeCount = 0;
        layoutInfo.pPushConstantRanges = nullptr;

        if(vkCreatePipelineLayout(device.getDevice(), &layoutInfo, nullptr, &cullPipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create compute pipeline layout.");
    }

    void RenderSystem::createComputePipeline(){
        assert(cullPipelineLayout != nullptr && "Cannot create compute pipeline before compute pipeline layout.");

        cullPipeline = std::make_unique<ComputePipeline>(
            device,
            "C:/Programming/C++_Projects/renderer/source/spirv_shaders/cull.comp.spv",
            cullPipelineLayout
        );
    }

    void RenderSystem::createIndirectCommands(){
        indirectCommands.clear();

        // TODO: sort through models that don't have indices and create commands for them and draw them seperately.
        // TODO: glTF models may have multiple nodes with different meshes; may need to have multiple commands per object.
        uint32_t totalInstanceCount = 0;
        uint32_t previousInstanceCount = 0;
        for(size_t i = 0; i < scene.models.size(); i++){
            uint32_t instanceCount = 0;
            // Get the number of objects that use this model, this will become the number of instances of this model.
            for(size_t j = 0; j < scene.objects.size(); j++){
                if(scene.objects.at(j).modelId == scene.models.at(i)->getId())
                    instanceCount++;
            }
            // Create a new indexedIndirectCommand for each unique model
            VkDrawIndexedIndirectCommand newIndexedIndirectCommand;
            newIndexedIndirectCommand.firstIndex = 0; // Currently there's one mesh per object so this will always be 0.
            newIndexedIndirectCommand.instanceCount = instanceCount; // Number of objects that use this unique model
            newIndexedIndirectCommand.firstInstance = previousInstanceCount; // Number of objects that used the previous model in the loop (offset by that number)
            newIndexedIndirectCommand.indexCount = scene.models.at(scene.objects.at(i).modelId)->getIndexCount(); // Number of indices the unique model has
            indirectCommands.push_back(newIndexedIndirectCommand); // Add the new command to the vector

            previousInstanceCount = instanceCount; // Updated at the end so the next command will use the current instanceCount value.
            totalInstanceCount += instanceCount; // Add to the total instance count the current number of instances
        }

        // Send indirect commands to GPU memory
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

        indirectCommandsBuffer = std::make_unique<Buffer>(
            device,
            1,
            stagingBuffer.getSize(),
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        stagingBuffer.copyBuffer(indirectCommandsBuffer->getBuffer(), indirectCommandsBuffer->getSize());
    }

    void RenderSystem::setupInstanceData(){
        instanceCullData.resize(totalInstanceCount);
        
        // Info set once as for all objects as the default, this info can be updated in the updateScene() function.
        for(uint32_t i = 0; i < totalInstanceCount; i++){
            auto object = scene.objects.at(i);
            instanceCullData[i].instanceIndex = object.getId(); // since we have one instance per object, instance index == objectId
            instanceCullData[i].indirectCommandID = object.modelId; // since we are creating one indirect command per model, indirect command id = object.modelId
        }
        // Two instance data buffers needed with duplicate data since we're double buffering
        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++){
            Buffer stagingBuffer{
                device,
                1,
                instanceCullData.size() * sizeof(InstanceCullData),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            };

            stagingBuffer.map();
            stagingBuffer.writeToBuffer(instanceCullData.data());

            instanceCullBuffers[i] = std::make_unique<Buffer>(
                device,
                1,
                stagingBuffer.getSize(),
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );

            stagingBuffer.copyBuffer(instanceCullBuffers[i]->getBuffer(), instanceCullBuffers[i]->getSize());
        }
    }

    void RenderSystem::drawScene(VkCommandBuffer commandBuffer, uint32_t frameIndex){
        cullPipeline->bind(commandBuffer);
        renderPipeline->bind(commandBuffer);
        vkCmdDrawIndexedIndirect(commandBuffer, indirectCommandsBuffer->getBuffer(), 0, static_cast<uint32_t>(indirectCommands.size()), sizeof(VkDrawIndexedIndirectCommand));
    }

    void RenderSystem::updateUniformBuffer(Camera camera, uint32_t frameIndex){
        // TODO: add check to see if camera view changed so needless updates are not performed
        uniformData.projection = camera.getProjection();
        uniformData.view = camera.getView();
        uniformData.inverseView = camera.getInverseView();

        uniformBuffers[frameIndex]->writeToBuffer(&uniformData);
        uniformBuffers[frameIndex]->flush();

        // TODO: move per-instance updates to the GPU as this method here is very slow when there is a large number of objects
        // Update per-instance data
        /*for(int i = 0; i < objectCount; i++){
            auto object = scene.objects.at(i);

            instanceData[i].modelMatrix = object.transform.mat4();
            instanceData[i].normalMatrix = object.transform.normalMatrix();
            
            instanceBuffers[frameIndex]->writeToBuffer(&instanceData[i]);
            instanceBuffers[frameIndex]->flush();
        }*/
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
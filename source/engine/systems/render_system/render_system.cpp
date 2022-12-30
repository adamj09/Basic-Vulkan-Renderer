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
    : device{device}, renderPass{renderPass}{}

    RenderSystem::~RenderSystem(){
        vkDestroyDescriptorSetLayout(device.getDevice(), globalSetLayout->getLayout(), nullptr);
        vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
    }

    void RenderSystem::initializeRenderSystem(){
        setupScene();
        setupDescriptorSets();

        createGraphicsPipelineLayout();
        createGraphicsPipeline();

        createComputePipelineLayout();
        createComputePipeline();

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

        // spongebob material
        scene.createMaterial();
        scene.materials.end()->second.diffuseTextureIds.push_back(0);

        // spongebob mesh
        scene.createMesh();
        scene.meshes.end()->second.modelId = 0; // spongebob model
        scene.meshes.end()->second.materialId = 0; // spongebob material

        // spongebob object
        scene.createObject();
        scene.objects.end()->second.transform.translation = {1.5f, .5f, 0.f};
        scene.objects.end()->second.transform.rotation = {glm::radians(180.f), 0.f, 0.f};
        scene.objects.end()->second.meshIds.push_back(0);

        // sample material
        scene.createMaterial();
        scene.materials.end()->second.diffuseTextureIds.push_back(1);

        // sample mesh
        scene.createMesh();
        scene.meshes.end()->second.modelId = 1;
        scene.meshes.end()->second.materialId = 1;

        // sample object
        scene.createObject();
        scene.objects.end()->second.transform.translation = {-.5f, .5f, 0.f};
        scene.objects.end()->second.transform.scale = {4.f, 4.f, 4.f};
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
        globalPool->addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT);        // Uniform data
        globalPool->buildPool(SwapChain::MAX_FRAMES_IN_FLIGHT);
        // Layout Setup
        globalSetLayout = std::make_unique<DescriptorSetLayout>(device);
        // Bindings are set in order of when they are added
        globalSetLayout->addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS);    // binding 0 (Uniform data)
        globalSetLayout->buildLayout();

        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++){
            // Fill universal matrix buffer info
            VkDescriptorBufferInfo uniformDataInfo = uniformBuffers[i]->descriptorInfo();

            // Writes list
            std::vector<VkWriteDescriptorSet> writes{
                globalSetLayout->writeBuffer(0, &uniformDataInfo), 
            };

            globalPool->allocateSet(globalSetLayout->getLayout());
            globalPool->updateSet(i, writes);
        }
    }

    void RenderSystem::createGraphicsPipelineLayout(){
        auto layout = globalSetLayout->getLayout();
        VkPipelineLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &layout;
        layoutInfo.pushConstantRangeCount = 0;
        layoutInfo.pPushConstantRanges = nullptr;

        if(vkCreatePipelineLayout(device.getDevice(), &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics pipeline layout.");
    }

    void RenderSystem::createGraphicsPipeline(){
        assert(pipelineLayout != nullptr && "Cannot create graphics pipeline before graphics pipeline layout.");

        GraphicsPipelineConfigInfo configInfo = {};
        GraphicsPipeline::defaultPipelineConfigInfo(configInfo);
        configInfo.pipelineLayout = pipelineLayout;
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
            "C:/Programming/C++_Projects/renderer/source/shaders/cull.comp",
            cullPipelineLayout
        );
    }

    void RenderSystem::createIndirectCommands(){
        instanceCount = static_cast<uint32_t>(scene.objects.size());

        // Where I left off, need to finish instanced rendering and indirect drawing + gpu-based culling
        // TODO: sort through models that don't have indices and create commands for them and draw them seperately.
        for(auto obj : scene.objects){
            for(int i = 0; i < obj.second.meshIds.size(); i++){
                VkDrawIndexedIndirectCommand newIndexedIndirectCommand;
                newIndexedIndirectCommand.firstIndex = 0;
                newIndexedIndirectCommand.instanceCount = instanceCount;
                newIndexedIndirectCommand.firstInstance = i * instanceCount;
                newIndexedIndirectCommand.indexCount = scene.models.at(scene.meshes.at(i).modelId)->getIndexCount();
                indirectCommands.push_back(newIndexedIndirectCommand);
            }
        }

        objectCount = 0;
        for(auto indCmd : indirectCommands)
            objectCount += indCmd.indexCount;

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
        instanceData.resize(objectCount);

        // Info set once as for all objects as the default, this info can be updated in the updateScene() function.
        /*for(uint32_t i = 0; i < objectCount; i++){
            auto object = scene.objects.at(i);
            instanceData[i].modelMatrix = object.transform.mat4();
            instanceData[i].normalMatrix = object.transform.normalMatrix();
            instanceData[i].translation = object.transform.translation;
            instanceData[i].scale = object.transform.scale;
            instanceData[i].rotation = object.transform.rotation;
            instanceData[i].modelId = object.modelId;
            instanceData[i].materialId = object.materialId;
        }*/
        // Two instance data buffers needed with duplicate data since we're double buffering
        for(int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++){
            Buffer stagingBuffer{
                device,
                1,
                instanceData.size() * sizeof(InstanceData),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            };

            stagingBuffer.map();
            stagingBuffer.writeToBuffer(instanceData.data());

            instanceBuffers[i] = std::make_unique<Buffer>(
                device,
                1,
                stagingBuffer.getSize(),
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );

            stagingBuffer.copyBuffer(instanceBuffers[i]->getBuffer(), instanceBuffers[i]->getSize());
        }
    }

    void RenderSystem::drawScene(VkCommandBuffer commandBuffer, uint32_t frameIndex){
        renderPipeline->bind(commandBuffer);
        vkCmdDrawIndexedIndirect(commandBuffer, indirectCommandsBuffer->getBuffer(), 0, static_cast<uint32_t>(indirectCommands.size()), sizeof(VkDrawIndexedIndirectCommand));
    }

    void RenderSystem::updateUniformBuffer(Camera camera, uint32_t frameIndex){
        // TODO: add check to see if camera view changed so needless updates are not performed
        uniformData.projection = camera.getProjection();
        uniformData.view = camera.getView();
        uniformData.inverseView = camera.getInverseView();
        uniformData.enableFrustumCulling = camera.enableFrustumCulling;

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
#include "scene.hpp"

#include <cassert>

namespace Renderer{
    Scene::Scene(){}

    void Scene::save(){

    }

    void Scene::load(){

    }

    void Scene::loadModels(Device& device){
        std::shared_ptr<Renderer::Model> spongebob = Renderer::Model::createModelFromFile(device, "../source/models/spongebob.obj");
        models[spongebob->getId()] = spongebob;

        std::shared_ptr<Renderer::Model> smoothVase = Renderer::Model::createModelFromFile(device, "../source/models/smooth_vase.obj");
        models[smoothVase->getId()] = smoothVase;
    }

    void Scene::loadTexturesWithSampler(Device& device, unsigned int samplerId){
        assert(samplers.count(samplerId) != 0 && "No sampler with given ID exists.");
        std::shared_ptr<Renderer::Texture> spongeTexture = Renderer::Texture::createTextureFromFile(device, "../source/textures/spongebob/spongebob.png");
        spongeTexture->samplerId = samplerId;
        textures[spongeTexture->getId()] = spongeTexture;

        std::shared_ptr<Renderer::Texture> sampleImage = Renderer::Texture::createTextureFromFile(device, "../source/textures/milkyway.jpg");
        sampleImage->samplerId = samplerId;
        textures[sampleImage->getId()] = sampleImage;
    }

    void Scene::createObject(){
        Object newObject = Object::createObject();
        objects.emplace(newObject.getId(), newObject);
    }

    void Scene::createSampler(Device& device, Sampler::SamplerConfig config){
        std::shared_ptr<Sampler> newSampler = Sampler::createSampler(device, config);
        samplers[newSampler->getId()] = newSampler;
    }

    void Scene::drawScene(){
        //vkCmdDrawIndexedIndirect();
    }

    void Scene::createDrawIndirectCommands(){
        drawIndexedCommands.clear();
        int totalInstanceCount = 0;

        for(size_t i = 0; i < models.size(); i++){
            uint32_t instanceCount = 0;
            // Get the number of objects that use this model, this will become the number of instances of this model.
            for(size_t j = 0; j < objects.size(); j++)
                if(objects.at(j).modelId == models.at(i)->getId())
                    instanceCount++;
        
            // Create a new indexedIndirectCommand for each unique model
            if(models.at(i)->getIndexCount() != 0){
                VkDrawIndexedIndirectCommand newIndexedIndirectCommand;
                newIndexedIndirectCommand.firstIndex = 0; // Currently there's one mesh per object so this will always be 0.
                newIndexedIndirectCommand.instanceCount = instanceCount; // Number of objects that use this unique model
                newIndexedIndirectCommand.firstInstance = 0; // Should always be 0 at the moment (start draw command at first instance of this model)
                newIndexedIndirectCommand.vertexOffset = 0; // Should always be 0
                newIndexedIndirectCommand.indexCount = models.at(objects.at(i).modelId)->getIndexCount(); // Number of indices the unique model has
                drawIndexedCommands.push_back(newIndexedIndirectCommand); // Add the new command to the vector
            }
            else {
                VkDrawIndirectCommand newIndirectCommand;
                newIndirectCommand.instanceCount = instanceCount; // Number of objects that use this unique model
                newIndirectCommand.firstInstance = 0; // Should always be 0 at the moment (start draw command at first instance of this model)
                newIndirectCommand.firstVertex = 0; // Should always be 0 at the moment
                newIndirectCommand.vertexCount = models.at(objects.at(i).modelId)->getVertexCount(); // Number of vertices the unique model has
                drawCommands.push_back(newIndirectCommand); // Add the new command to the vector
            }
            totalInstanceCount += instanceCount; // Add to the total instance count to the current number of instances
        }
    }
}
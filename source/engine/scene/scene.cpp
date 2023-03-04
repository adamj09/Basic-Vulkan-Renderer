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
        sceneUniform.instanceCount++;
    }

    void Scene::createSampler(Device& device, Sampler::SamplerConfig config){
        std::shared_ptr<Sampler> newSampler = Sampler::createSampler(device, config);
        samplers[newSampler->getId()] = newSampler;
    }
}
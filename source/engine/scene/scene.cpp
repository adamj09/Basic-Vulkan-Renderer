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

        std::shared_ptr<Renderer::Texture> spongebobTexture = Renderer::Texture::createTextureFromFile(device, "../source/textures/spongebob/spongebob.png");
        spongebobTexture->samplerId = samplerId;
        textures[spongebobTexture->getId()] = spongebobTexture;

        std::shared_ptr<Renderer::Texture> sampleTexture = Renderer::Texture::createTextureFromFile(device, "../source/textures/milkyway.jpg");
        sampleTexture->samplerId = samplerId;
        textures[sampleTexture->getId()] = sampleTexture;
    }

    void Scene::createObject(){
        Object newObject = Object::createObject();
        objects.emplace(newObject.getId(), newObject);
    }

    void Scene::createSampler(Device& device, Sampler::SamplerConfig config){
        std::shared_ptr<Sampler> newSampler = Sampler::createSampler(device, config);
        samplers[newSampler->getId()] = newSampler;
    }
}
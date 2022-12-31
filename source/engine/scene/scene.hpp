#pragma once

#include "engine/object/object.hpp"
#include "engine/mesh/mesh.hpp"
#include "engine/mesh/model.hpp"
#include "engine/material/texture/texture.hpp"
#include "engine/material/sampler/sampler.hpp"

#include <unordered_map>

namespace Renderer{
    class Scene{
        public:
            Scene();

            void save();
            void load();

            void loadModels(Device& device);
            void loadTexturesWithSampler(Device& device, unsigned int samplerId);

            void createObject();
            void createMesh();
            void createMaterial();

            void createSampler(Device& device, Sampler::SamplerConfig config);

            // Total objects in scene, can contain copies
            Object::Map objects;

            // Unique meshes and materials (to be instanced)
            Mesh::Map meshes;
            Material::Map materials;

            // Samplers (created by user indirectly and can be shared between textures)
            std::unordered_map<unsigned int, std::shared_ptr<Sampler>> samplers;

            // Raw assets (loaded from files the user specifies)
            std::unordered_map<unsigned int, std::shared_ptr<Model>> models;
            std::unordered_map<unsigned int, std::shared_ptr<Texture>> textures;
    };
}
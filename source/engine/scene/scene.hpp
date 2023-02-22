#pragma once

#include "engine/object/object.hpp"
#include "engine/model/model.hpp"
#include "engine/material/texture/texture.hpp"
#include "engine/material/sampler/sampler.hpp"
#include "engine/material/material.hpp"
#include "engine/camera/camera.hpp"

#include <unordered_map>

namespace Renderer{
    class Scene{
        public:
            struct SceneUniform{  
                glm::mat4 projection{1.f};
                glm::mat4 view{1.f};
                glm::mat4 inverseView{1.f};

                BoundingBox viewBoundingBox;

                bool enableOcclusionCulling = true;
                bool enableFrustumCulling = true;

                uint32_t instanceCount = 0;
            } sceneUniform;

            Scene();

            void save();
            void load();

            void loadModels(Device& device);
            void loadTexturesWithSampler(Device& device, unsigned int samplerId);

            void createObject();
            void createSampler(Device& device, Sampler::SamplerConfig config);

            // Total objects in scene, can contain copies
            Object::Map objects;

            // Samplers, does not contain copies (created by user indirectly and can be shared between textures)
            std::unordered_map<unsigned int, std::shared_ptr<Sampler>> samplers;

            // Raw assets, do not contain copies (loaded from files the user specifies)
            std::unordered_map<unsigned int, std::shared_ptr<Model>> models;
            std::unordered_map<unsigned int, std::shared_ptr<Texture>> textures;

            std::vector<VkDrawIndexedIndirectCommand> drawIndexedCommands;
            std::vector<VkDrawIndirectCommand> drawCommands;
    };
}
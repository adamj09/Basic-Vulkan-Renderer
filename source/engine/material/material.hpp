#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "engine/device/device.hpp"
#include "engine/material/texture/texture.hpp"

#include <vector>
#include <unordered_map>

namespace Renderer{
    class Material{
        public:
            struct MaterialProperties{
                float opacity = 1.0f;   // Opacity of the material applied.
                float shininess = 0.0f; // How intense specular reflections will be.

                glm::vec4 diffuseColour = { 1.0f, 1.0f, 1.0f, 1.0f };   // Colour of diffuse reflections
                glm::vec4 specularColour = { 1.0f, 1.0f, 1.0f, 1.0f };  // Colour of specular reflections
                glm::vec4 hue = { 1.0f, 1.0f, 1.0f, 1.0f };             // Overall hue of the material
            } properties{};

            using Map = std::unordered_map<unsigned int, Material>;

            static Material createMaterial(){
                static unsigned int currentId = 0;
                return Material{currentId++};
            }
    
            unsigned int getId() { return id; }

            std::vector<unsigned int> diffuseTextureIds;
            std::vector<unsigned int> normalTextureIds;

        private:
            Material(unsigned int materialId) : id{materialId} {}

            unsigned int id;
    };
}
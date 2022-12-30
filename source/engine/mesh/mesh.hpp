#pragma once

#include "engine/device/device.hpp"
#include "engine/material/material.hpp"

namespace Renderer{
    class Mesh{
        public:
            struct PointLightComponent{
                bool emitLight = false;         // Choose whether to emit light, if false, all variables below are irrelevant.

                float brightness = 0.0f;        // How bright a light is, its intensity.
                float width = 0.0f;             // Width of the light beam.

                glm::vec3 lightDirection = { 0.0f, 0.0f, 0.0f };    // Vector describing the direction the rays of light are going relative to the source.
                glm::vec4 hue = { 1.0f, 1.0f, 1.0f, 1.0f };         // Hue of the light.
            } pointLightComponent{};

            using Map = std::unordered_map<unsigned int, Mesh>;

            static Mesh createMesh(){
                static unsigned int currentId = 0;
                return Mesh{currentId++};
            };

            unsigned int getId() { return id; }

            unsigned int modelId;
            unsigned int materialId;

        private:
            Mesh(unsigned int meshId) : id{meshId} {};
            
            unsigned int id;
    };
}
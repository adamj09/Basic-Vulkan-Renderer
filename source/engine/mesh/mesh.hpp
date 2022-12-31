#pragma once

#include "engine/device/device.hpp"
#include "engine/material/material.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Renderer{
    //TODO: create matrix transformation class or something to seperate it from the mesh file (inapropriate location at the moment).
    struct TransformComponent {
        glm::vec3 translation{};    // "Position" of the mesh
        glm::vec3 scale{1.f, 1.f, 1.f}; // Each value here is a multiplier applied to different axis of the mesh
        glm::vec3 rotation{};   // Rotation of the mesh in radians

        glm::mat4 mat4();   // model matrix
        glm::mat3 normalMatrix(); // normal matrix (used for lighting)
    };
    //TODO: add point light object to seperate light functionality from meshes.
    struct PointLightComponent{
        bool emitLight = false;         // Choose whether to emit light, if false, all variables below are irrelevant.
        float brightness = 0.0f;        // How bright a light is, its intensity.
        float width = 0.0f;             // Width of the light beam.

        glm::vec3 lightDirection = { 0.0f, 0.0f, 0.0f };    // Vector describing the direction the rays of light are going relative to the source.
        glm::vec4 hue = { 1.0f, 1.0f, 1.0f, 1.0f };         // Hue of the light.
    };

    class Mesh{
        public:
            using Map = std::unordered_map<unsigned int, Mesh>;

            static Mesh createMesh(){
                static unsigned int currentId = 0;
                return Mesh{currentId++};
            };

            TransformComponent transform{};
            PointLightComponent pointLight{};

            unsigned int getId() { return id; }

            unsigned int modelId;
            unsigned int materialId;

        private:
            Mesh(unsigned int meshId) : id{meshId} {};
            
            unsigned int id;
    };
}
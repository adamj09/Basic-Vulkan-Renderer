#pragma once

#include "engine/model/model.hpp"
#include "engine/material/texture/texture.hpp"
#include "engine/material/sampler/sampler.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace Renderer{
    //TODO: maybe add a point light object to separate light functionality from objects.

    class Object{
        public:
            struct TransformComponent {
                glm::vec3 translation{};    // "Position" of the mesh
                glm::vec3 scale{1.f, 1.f, 1.f}; // Each value here is a multiplier applied to different axis of the mesh
                glm::vec3 rotation{};   // Rotation of the mesh in radians

                glm::mat4 mat4();   // model matrix
                glm::mat3 normalMatrix(); // normal matrix (used for lighting)
            } transform;

            struct PointLightComponent{
                float brightness = 0.0f;        // How bright a light is, its intensity.
                float width = 0.0f;             // Width of the light beam.

                glm::vec3 lightDirection = { 0.0f, 0.0f, 0.0f };    // Vector describing the direction the rays of light are going relative to the source.
                glm::vec4 hue = { 1.0f, 1.0f, 1.0f, 1.0f };         // Hue of the light.
            } pointLight;

            struct ObjectInfo{
                unsigned int modelId;
                unsigned int diffuseId;

                unsigned int boundingSphereRadius;
                glm::vec3 boundingSphereCenter;

                glm::mat4 modelMatrix{1.f};
                glm::mat3 normalMatrix{1.f};
            } objectInfo;

            using Map = std::unordered_map<unsigned int, Object>;

            static Object createObject() {
                static unsigned int currentId = 0;
                return Object{currentId++};
            }

            unsigned int getId() { return id; }

        private:
            Object(unsigned int objectId) : id{objectId} {}

            unsigned int id;
    };
}
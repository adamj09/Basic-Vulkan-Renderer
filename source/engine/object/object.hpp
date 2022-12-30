#pragma once

#include "engine/mesh/model.hpp"
#include "engine/material/texture/texture.hpp"
#include "engine/material/sampler/sampler.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace Renderer{
    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{};
        glm::mat4 mat4();

        glm::mat3 normalMatrix();
    };

    class Object{
        public:
            using Map = std::unordered_map<unsigned int, Object>;

            static Object createObject() {
                static unsigned int currentId = 0;
                return Object{currentId++};
            }

            unsigned int getId() { return id; }
            
            TransformComponent transform{};

            std::vector<unsigned int> meshIds;

        private:
            Object(unsigned int objectId) : id{objectId} {}

            unsigned int id;
    };
}
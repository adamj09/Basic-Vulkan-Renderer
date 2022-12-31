#pragma once

#include "engine/mesh/model.hpp"
#include "engine/material/texture/texture.hpp"
#include "engine/material/sampler/sampler.hpp"
#include "engine/mesh/mesh.hpp"

#include <memory>
#include <unordered_map>

namespace Renderer{
    class Object{
        public:
            using Map = std::unordered_map<unsigned int, Object>;

            static Object createObject() {
                static unsigned int currentId = 0;
                return Object{currentId++};
            }

            unsigned int getId() { return id; }
            
            std::vector<unsigned int> meshIds;

        private:
            Object(unsigned int objectId) : id{objectId} {}

            unsigned int id;
    };
}
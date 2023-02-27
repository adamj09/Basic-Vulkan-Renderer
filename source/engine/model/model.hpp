#pragma once

#include "engine/buffer/buffer.hpp"
#include "glm/glm.hpp"

#include <memory>
#include <unordered_map>

namespace Renderer{
    // Class representing a 3D model
    class Model{
        public:
            struct Vertex {
                glm::vec3 position{};
                glm::vec3 colour{};
                glm::vec3 normal{};
                glm::vec2 texCoords{};

                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            
                bool operator==(const Vertex &other) const {
                    return position == other.position && colour == other.colour && normal == other.normal && texCoords == other.texCoords;
                }
            };

            struct ModelData{
                std::vector<Vertex> vertices{};
                std::vector<uint32_t> indices{};
                void loadModel(const std::string &filepath);
            };

            Model(Device& device, ModelData& data, unsigned int modelId);

            Model(const Model&) = delete;
            Model &operator=(const Model&) = delete;
        
            unsigned int getId() { return modelId; }
            static std::unique_ptr<Model> createModelFromFile(Device& device, const std::string& filepath);

            std::vector<Vertex> getVertices() { return modelData.vertices; }
            std::vector<uint32_t> getIndices() { return modelData.indices; }

            uint32_t getVertexCount() { return static_cast<uint32_t>(modelData.vertices.size()); }
            uint32_t getIndexCount() { return static_cast<uint32_t>(modelData.indices.size()); }
            
        private:    
            Device& device;
            ModelData& modelData;

            unsigned int modelId;
    };
}
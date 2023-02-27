#include "model.hpp"

#include "engine/utils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <cassert>
#include <unordered_map>

namespace std{
    template <>
    struct hash<Renderer::Model::Vertex>{
        size_t operator()(Renderer::Model::Vertex const& vertex) const{
            size_t seed = 0;
            Renderer::hashCombine(seed, vertex.position, vertex.colour, vertex.normal, vertex.texCoords);
            return seed;
        }
    };
}

namespace Renderer{
    Model::Model(Device& device, ModelData& data, unsigned int modelId) : device{device}, modelData{modelData}, modelId{modelId}{}

    std::unique_ptr<Model> Model::createModelFromFile(Device& device, const std::string& filepath){
        static unsigned int currentId = 0;
        ModelData data{};
        data.loadModel(filepath);
        return std::make_unique<Model>(device, data, currentId++);
    }

    void Model::ModelData::loadModel(const std::string &filepath){
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {
                Vertex vertex{};
                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };
                    vertex.colour = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                }
                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }
                if (index.texcoord_index >= 0) {
                    vertex.texCoords = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }
                if (uniqueVertices.count(vertex) == 0) {
                  uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                  vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }     
        }
    }

    std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions(){
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions(){
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);
        attributeDescriptions[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)};    // Vertex position
        attributeDescriptions[1] = {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, colour)};      // Vertex Colour
        attributeDescriptions[2] = {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)};      // Normals
        attributeDescriptions[3] = {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoords)};      // TexCoords/UV
        return attributeDescriptions;
    }
}
#pragma once

#include "engine/device/device.hpp"
#include "engine/buffer/buffer.hpp"
#include "engine/material/sampler/sampler.hpp"

#include <unordered_map>
#include <memory>

namespace Renderer{
    class Texture{
        public: 
            Texture(Device& device, std::string filepath, unsigned int textureId);
            ~Texture();

            Texture(const Texture&) = delete;
            Texture &operator=(const Texture&) = delete;

            static std::unique_ptr<Texture> createTextureFromFile(Device& device, std::string filepath);

            VkImageView getTextureImageView() { return textureImageView; }
            uint32_t getMipLevels() { return mipLevels; }
            VkDescriptorImageInfo descriptorImageInfo();
            unsigned int getId() { return textureId; }

            unsigned int samplerId;

        private:
            void createTexture(std::string filepath);
            void createTextureImage();
            void createTextureImageView();
            void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
            void generateMipmaps();

            Device& device;

            VkImage textureImage;
            VkImageView textureImageView;
            VkDeviceMemory textureImageMemory;
            uint32_t mipLevels;

            VkExtent2D imageExtent;
            std::unique_ptr<Buffer> imageBuffer;

            unsigned int textureId;
    };
}
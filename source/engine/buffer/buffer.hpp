#pragma once

#include "engine/device/device.hpp"

namespace Renderer{
    class Buffer{
        public:
            Buffer(Device& device, uint32_t count, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkSharingMode sharingMode,
                VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
            ~Buffer();

            Buffer(const Buffer&) = delete;
            Buffer& operator=(const Buffer&) = delete;

            VkBuffer getBuffer(){ return buffer; }
            VkDeviceSize getSize() { return bufferSize; }

            VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
            void unmap();
            
            void writeToBuffer(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
            void copyBuffer(VkBuffer dstBuffer, VkDeviceSize size);
            void copyBufferToImage(VkImage image, uint32_t width, uint32_t height);
            VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
            VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        private:
            void createbuffer();

            Device& device;
            
            void* mapped = nullptr;
            VkBuffer buffer = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;

            VkDeviceSize bufferSize;
            VkBufferUsageFlags usage;
            uint32_t count;
            VkMemoryPropertyFlags memoryPropertyFlags; 
            VkDeviceSize alignmentSize;
            VkSharingMode sharingMode;

            static VkDeviceSize getAlignment(VkDeviceSize size, VkDeviceSize minOffsetAlignment);
    };
}
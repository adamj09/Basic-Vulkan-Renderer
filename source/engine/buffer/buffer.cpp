#include "buffer.hpp"

#include <stdexcept>
#include <cassert>

namespace Renderer{
    Buffer::Buffer(Device& device, uint32_t count, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkSharingMode sharingMode,
        VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment) 
    : device{device}, count{count}, bufferSize{bufferSize}, usage{usage}, sharingMode{sharingMode}, memoryPropertyFlags{memoryPropertyFlags}{
        alignmentSize = getAlignment(bufferSize, minOffsetAlignment);
        bufferSize = alignmentSize * count;
        createbuffer();
    }

    Buffer::~Buffer(){
        unmap();
        vkDestroyBuffer(device.getDevice(), buffer, nullptr);
        vkFreeMemory(device.getDevice(), memory, nullptr);
    }

    void Buffer::createbuffer(){
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = sharingMode;

        if(vkCreateBuffer(device.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to create buffer.");
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device.getDevice(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, memoryPropertyFlags);

        if (vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate vertex buffer memory.");
        vkBindBufferMemory(device.getDevice(), buffer, memory, 0);
    }

    VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset){
        assert(buffer && memory && "Called map on buffer before create.");
        return vkMapMemory(device.getDevice(), memory, offset, size, 0, &mapped);
    }

    void Buffer::unmap(){
        if(mapped){
            vkUnmapMemory(device.getDevice(), memory);
            mapped = nullptr;
        }
    }

    void Buffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset){
        assert(mapped && "Cannot copy to unmapped buffer.");
        if (size == VK_WHOLE_SIZE)
            memcpy(mapped, data, bufferSize);
        else {
            char *memOffset = (char *)mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    void Buffer::copyBuffer(VkBuffer dstBuffer, VkDeviceSize size){
        VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, buffer, dstBuffer, 1, &copyRegion);

        device.endSingleTimeCommands(commandBuffer);
    }

    void Buffer::copyBufferToImage(VkImage image, uint32_t width, uint32_t height){
        VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        device.endSingleTimeCommands(commandBuffer);
    }

    VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(device.getDevice(), 1, &mappedRange);
    }

    VkDeviceSize Buffer::getAlignment(VkDeviceSize size, VkDeviceSize minOffsetAlignment){
        if (minOffsetAlignment > 0)
            return (size + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        return size;
    }

    VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        return VkDescriptorBufferInfo{buffer, offset, size};
    }
}
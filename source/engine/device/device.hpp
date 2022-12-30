#pragma once

#include "vulkan/vulkan.h"

#include "engine/window/window.hpp"
#include "engine/debugging/vulkan_debugger.hpp"

#include <vector>

namespace Renderer{
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
	    std::vector<VkSurfaceFormatKHR> formats;
	    std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices{
        uint32_t graphicsFamily, presentFamily;
	    bool graphicsFamilyHasValue = false, presentFamilyHasValue = false;
	    bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    class Device{
        public:
            Device(Window& window);
            ~Device();

            // Getter Functions
            VkDevice getDevice() { return device; }
            VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
            VkSurfaceKHR getSurface() { return surface; }
            SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
            QueueFamilyIndices getPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
            VkCommandPool getCommandPool(){ return commandPool; }
            VkQueue getGraphicsQueue() { return graphicsQueue; }
            VkQueue getPresentQueue() { return presentQueue; }
            VkSampleCountFlagBits getMaxUsableSampleCount();
            

            // Other Public Functions
            VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
            void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            VkCommandBuffer beginSingleTimeCommands();
            void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        private:
            // Initialize (call all main functions)
            void initVulkan();
            // Main Functions
            void createInstance();
            void createSurface();
            void pickPhysicalDevice();
            void createLogicalDevice();
            void createCommandPool();

            // Helper Functions
            std::vector<const char*> getRequiredExtensions();
            bool isDeviceSuitable(VkPhysicalDevice device);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
            bool checkDeviceExtensionSupport(VkPhysicalDevice device);
            void hasRequiredExtensions();

            VkInstance instance;
            VkDevice device;
            VkPhysicalDevice physicalDevice;
            VkPhysicalDeviceProperties properties;
            VkSurfaceKHR surface;
            Window& window;
            VkQueue graphicsQueue, presentQueue;
            VkCommandPool commandPool;

            Debugger::VulkanDebugger debugger;

            // Expand this vector to include all needed device extensions
            const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    };
}
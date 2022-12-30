#pragma once
#include "vulkan/vulkan.h"

#include <vector>

namespace Debugger{
    // A class that handles vulkan debugging tools, for now only the validation layers provided by Khronos
    class VulkanDebugger{
        public:
            VulkanDebugger();
            ~VulkanDebugger();

            #ifdef NDEBUG
                static const bool enableValidationLayers = false;
            #else
                static const bool enableValidationLayers = true;
            #endif

            const std::vector<const char*> getValidationLayers() { return validationLayers; }
            VkDebugUtilsMessengerEXT getDebugMessenger() { return debugMessenger; }

            void setupDebugMessenger(VkInstance instance);
            void destroyDebugUtilsMessengerEXT(VkInstance instance, const VkAllocationCallbacks* pAllocator);
            static VkDebugUtilsMessengerCreateInfoEXT defaultDebugMessengerCreateInfo();

        private:
            static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
                const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
            VkDebugUtilsMessengerEXT debugMessenger;
            const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    };
}
#include "vulkan_debugger.hpp"

#include <iostream>
#include <stdexcept>

namespace Debugger{
    VulkanDebugger::VulkanDebugger(){}
    VulkanDebugger::~VulkanDebugger(){}

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << "\n\n";
        return VK_FALSE;
    }
    
    void VulkanDebugger::setupDebugMessenger(VkInstance instance){
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        createInfo = defaultDebugMessengerCreateInfo();
        if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug messenger.");
    }

    VkResult VulkanDebugger::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void VulkanDebugger::destroyDebugUtilsMessengerEXT(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }

    VkDebugUtilsMessengerCreateInfoEXT VulkanDebugger::defaultDebugMessengerCreateInfo(){
        VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugInfo.messageType = 
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        debugInfo.pfnUserCallback = debugCallback;
        debugInfo.pUserData = nullptr;
        debugInfo.pNext = nullptr;
        return debugInfo;
    }
}
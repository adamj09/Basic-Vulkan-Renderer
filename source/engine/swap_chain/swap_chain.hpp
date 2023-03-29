#pragma once

#include "engine/device/device.hpp"
#include <memory>

namespace Renderer{
    class SwapChain{
        public:
            SwapChain(Device& device, VkExtent2D windowExtent);
            SwapChain(Device& device, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
            ~SwapChain();

            static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

            // Getter functions
            VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
            VkExtent2D getSwapChainExtent() { return swapChainExtent; }
            size_t getImageCount() { return swapChainImages.size(); }
            VkRenderPass getRenderPass() { return renderPass; }
            VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
            float extentAspectRatio() { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }
            VkSwapchainKHR getSwapChain() { return swapChain; }

            // Other functions
            bool compareSwapFormats(const SwapChain& swapChain) const { return swapChain.swapChainDepthFormat == swapChainDepthFormat && 
                swapChain.swapChainImageFormat == swapChainImageFormat; }
            VkResult acquireNextImage(uint32_t* imageIndex);
            VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

        private:
            // Calls all main functions
            void initSwapChain();
            // Main Functions
            void createSwapChain();
            void createImageViews();
            void createColourResources();
            void createDepthResources();
            void createRenderPass();
            void createFramebuffers();
            void createSyncObjects();

            // Helper Functions
            VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> formats);
            VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& presentModes, VkPresentModeKHR desiredMode);
            VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
            VkImageView createImageView(VkImage image, VkFormat format, uint32_t mipLevels, VkImageAspectFlagBits imageAspect);
            VkFormat findDepthFormat();

            Device& device;
            VkExtent2D windowExtent;

            VkSwapchainKHR swapChain;

            std::shared_ptr<SwapChain> oldSwapChain;

            std::vector<VkFramebuffer> swapChainFramebuffers;

            std::vector<VkImage> colourImages;
            std::vector<VkDeviceMemory> colourImageMemories;
            std::vector<VkImageView> colourImageViews;

            std::vector<VkImage> depthImages;
            std::vector<VkDeviceMemory> depthImageMemories;
            std::vector<VkImageView> depthImageViews;

            std::vector<VkImage> swapChainImages;
            std::vector<VkImageView> swapChainImageViews;

            VkFormat swapChainImageFormat;
            VkFormat swapChainDepthFormat;
            VkExtent2D swapChainExtent;

            std::vector<VkSemaphore> imageAvailableSemaphores;
            std::vector<VkSemaphore> renderFinishedSemaphores;
            std::vector<VkFence> inFlightFences;
            std::vector<VkFence> imagesInFlight;
            size_t currentFrame = 0;

            VkRenderPass renderPass;
    };
}
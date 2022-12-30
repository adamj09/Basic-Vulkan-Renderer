#pragma once

#include "engine/device/device.hpp"
#include "engine/swap_chain/swap_chain.hpp"
#include "engine/window/window.hpp"
#include "engine/systems/render_system/render_system.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace Renderer{
    class Renderer{
        public:
            Renderer(Device& device, Window& window);
            ~Renderer();
            
            int getCurrentFrameIndex() { return currentFrameIndex; }
            VkRenderPass getSwapChainRenderPass() { return swapChain->getRenderPass(); }
            float getAspectRatio() const { return swapChain->extentAspectRatio(); }

            VkCommandBuffer getCurrentCommandBuffer() const {
                assert(isFrameStarted && "Cannot get command buffer when a frame is not in progress.");
                return commandBuffers[currentFrameIndex];
            }

            int getFrameIndex() const {
                assert(isFrameStarted && "Cannot get frame index when a frame is not in progress.");
                return currentFrameIndex;
            }

            VkCommandBuffer beginFrame();
            void endFrame();

            void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        private:
            void recreateSwapChain();
            void createCommandBuffers();
            void freeCommandBuffers();


            Device& device;
            Window& window;

            std::unique_ptr<SwapChain> swapChain;
            std::vector<VkCommandBuffer> commandBuffers;

            uint32_t currentImageIndex;
            int currentFrameIndex{0};
            bool isFrameStarted{false};
    };
}
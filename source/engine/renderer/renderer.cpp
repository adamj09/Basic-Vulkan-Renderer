#include "renderer.hpp"

#include <stdexcept>
#include <array>

namespace Renderer{
    Renderer::Renderer(Device& device, Window& window) : device{device}, window{window}{
        recreateSwapChain();
        createCommandBuffers();
    }

    Renderer::~Renderer(){
        freeCommandBuffers();
    }

    void Renderer::recreateSwapChain(){
        auto extent = window.getExtent();
        while(extent.width == 0 || extent.height == 0){
            extent = window.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(device.getDevice());
        if(swapChain == nullptr)
            swapChain = std::make_unique<SwapChain>(device, extent);
        else{
            std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
            swapChain = std::make_unique<SwapChain>(device, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*swapChain.get()))
                throw std::runtime_error("Swap chain image or depth format has changed.");
        }
    }

    void Renderer::createCommandBuffers(){
        commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo bufferAllocInfo = {};
        bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        bufferAllocInfo.commandPool = device.getCommandPool();
        bufferAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
        bufferAllocInfo.pNext = nullptr;

        if(vkAllocateCommandBuffers(device.getDevice(), &bufferAllocInfo, commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocated command buffers.");
    }

    void Renderer::freeCommandBuffers(){
        vkFreeCommandBuffers(device.getDevice(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    VkCommandBuffer Renderer::beginFrame(){
        assert(!isFrameStarted && "Can't call beginFrame() while already in progress.");
        auto result = swapChain->acquireNextImage(&currentImageIndex);
        
        if(result == VK_ERROR_OUT_OF_DATE_KHR){
            recreateSwapChain();
            return nullptr;
        }

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("Failed to acquire swap chain image.");

        isFrameStarted = true;
        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to begin recording command buffer.");
        return commandBuffer;
    }

    void Renderer::endFrame(){
        assert(isFrameStarted && "Can't call endFrame() when frame is not in progress.");
        auto commandBuffer = getCurrentCommandBuffer();
        if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to end command buffer.");

        auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if(result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || window.wasWindowResized()){
            window.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if(result != VK_SUCCESS)
            throw std::runtime_error("Failed to present swap chain image.");

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer){
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");
    
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapChain->getRenderPass();
        renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

        std::array<VkClearValue, 3> clearValues{};
        clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f }; // Default "background colour" rendered
        clearValues[1].depthStencil = { 1.0f, 0 }; // Default render depth
        clearValues[2].color = { 0.01f, 0.01f, 0.01f, 1.0f }; // Also default "background colour" rendered
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, swapChain->getSwapChainExtent() };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
        vkCmdEndRenderPass(commandBuffer);
    }
}
#pragma once

#include "engine/device/device.hpp"
#include "engine/swap_chain/swap_chain.hpp"
#include "engine/renderer/renderer.hpp"
#include "engine/object/object.hpp"

#include <vector>
#include <unordered_map>

namespace Application{
    // Class containing all essential, basic functions and variables needed to run the app
    class App{
        public:
            App();
            ~App();

            void run();
            void createObjects();
        private:

            VkExtent2D windowExtent = {1280, 720};
            Renderer::Window window{static_cast<int>(windowExtent.width), static_cast<int>(windowExtent.height), "Renderer View"};
            Renderer::Device device{window};
            Renderer::Renderer renderer{device, window};
            Renderer::RenderSystem renderSystem{device, renderer.getSwapChainRenderPass()};
    };
}
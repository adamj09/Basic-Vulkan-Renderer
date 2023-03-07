#include "app.hpp"

#include <stdexcept>
#include <chrono>
#include <iostream>
#include <unordered_map>

#include "engine/systems/render_system/render_system.hpp"
#include "engine/camera/camera.hpp"
#include "engine/camera/camera_controller/camera_controller.hpp"

namespace Application{
    App::App(){}
    App::~App(){}

    void App::run(){
        // Camera creation
        Renderer::Camera camera{};
        camera.enableFrustumCulling = true;
        Renderer::KeyboardMovementController cameraController{};
        auto viewerObject = Renderer::Object::createObject();
        viewerObject.transform.translation.z = -2.5f;

        float intervalTime = 0;
        auto currentTime = std::chrono::steady_clock::now();

        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &physicalDeviceProperties);

        std::cout << "Max compute work group count: " << physicalDeviceProperties.limits.maxComputeWorkGroupCount[0] << '\n';
        std::cout << "Max compute work group invocations: " << physicalDeviceProperties.limits.maxComputeWorkGroupInvocations << '\n';
        std::cout << "Max compute work group size: " << physicalDeviceProperties.limits.maxComputeWorkGroupSize[0] << '\n';

        while(!window.shouldClose()){
            glfwPollEvents();
            
            auto newTime = std::chrono::steady_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            intervalTime += frameTime;
            if(intervalTime >= 3000){
                std::cout << "Frametime: " << frameTime << " ms" << '\n';
                intervalTime = 0;
            }

            cameraController.moveSpeed = (0.0035f);
            cameraController.lookSpeed = (0.0035f);
            cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(90.f), aspect, 0.1f, 100.f);

            if (auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();
                // Update
                renderSystem.updateSceneUniform(camera, frameIndex);
                // Cull Scene
                //renderSystem.cullScene(commandBuffer, frameIndex);
                // Start Renderpass
                renderer.beginSwapChainRenderPass(commandBuffer);
                // Draw Objects
                renderSystem.drawScene(commandBuffer, frameIndex);
                // End Renderpass
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(device.getDevice());
    }
}
#include "app.hpp"

#include <stdexcept>
#include <chrono>
#include <iostream>
#include <unordered_map>

#include "engine/systems/render_system/render_system.hpp"
#include "engine/camera/camera.hpp"
#include "engine/camera/camera_controller/camera_controller.hpp"
#include "engine/material/texture/texture.hpp"
#include "engine/material/sampler/sampler.hpp"

namespace Application{
    App::App(){
        renderSystem.initializeRenderSystem();
    }

    App::~App(){}

    void App::run(){
        // Camera creation
        Renderer::Camera camera{};
        Renderer::KeyboardMovementController cameraController{};
        auto viewerObject = Renderer::Object::createObject();
        viewerObject.transform.translation.z = -2.5f;

        float intervalTime = 0;
        auto currentTime = std::chrono::steady_clock::now();

        while(!window.shouldClose()){
            glfwPollEvents();
            // Frametime Calculation
            auto newTime = std::chrono::steady_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            intervalTime += frameTime;
            if(intervalTime >= 3000){
                std::cout << "Frametime: " << frameTime << " ms" << '\n';
                intervalTime = 0;
            }

            // Camera Setup
            cameraController.moveSpeed = (0.0035f); //TODO: should probably add a "look sensitivity" option, also need to add mouse controls alongside existing keyboard controls
            cameraController.lookSpeed = (0.0035f);
            cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(90.f), aspect, 0.1f, 100.f);

            if (auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();
                // Update
                renderSystem.updateUniformBuffer(camera, frameIndex);
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
    // Test comment 2
    /*void App::createObjects(){
        //Sampler for testing
        Renderer::Sampler::SamplerConfig textureSamplerConfig{};
        textureSamplerConfig.anisotropyEnable = VK_TRUE;
        textureSamplerConfig.maxAnisotropy = 16.f;
        textureSamplerConfig.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        textureSamplerConfig.maxLod = 100.f;
        textureSampler = Renderer::Sampler::createSampler(device, textureSamplerConfig);

        // Materials for testing
        auto spongeMaterial = Renderer::Material::createMaterial();
        spongeMaterial.diffuseTextureId = spongeTexture->getId();
        scene.materials.emplace(spongeMaterial.getId(), spongeMaterial);

        auto sampleMaterial = Renderer::Material::createMaterial();
        sampleMaterial.diffuseTextureId = sampleImage->getId();
        scene.materials.emplace(sampleMaterial.getId(), sampleMaterial);

        auto sampleObject = Renderer::Object::createObject();
        sampleObject.modelId = spongebob->getId();
        sampleObject.materialId = spongeMaterial.getId();
        sampleObject.transform.translation = {1.5f, .5f, 0.f};
        sampleObject.transform.scale = {1.f, 1.f, 1.f};
        sampleObject.transform.rotation = {glm::radians(180.f), 0.f, 0.f};
        scene.objects.emplace(sampleObject.getId(), sampleObject);

        auto vase = Renderer::Object::createObject();
        vase.modelId = smoothVase->getId();
        vase.transform.translation = {-.5f, .5f, 0.f};
        vase.transform.scale = {4.f, 4.f, 4.f};
        vase.materialId = sampleMaterial.getId();
        scene.objects.emplace(vase.getId(), vase);
    }*/
}
#include "app.hpp"

#include <stdexcept>
#include <chrono>
#include <iostream>
#include <unordered_map>

#include "engine/systems/render_system/render_system.hpp"
#include "engine/camera/camera.hpp"

namespace Application{
    App::App(){}
    App::~App(){}

    void App::run(){
        // Camera creation
        float aspect = renderer.getAspectRatio();
        Renderer::Camera camera{};
        camera.enableFrustumCulling = true;

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

            camera.moveSpeed = (0.0035f);
            camera.lookSpeed = (0.0035f);
            camera.setPerspectiveProjection(glm::radians(90.f), aspect, 0.1f, 100.f);
            camera.moveInPlaneXZ(window.getGLFWwindow(), frameTime);

            if (auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();
                // Update
                renderSystem.updateSceneUniform(camera, frameIndex);
                // Cull Scene     
                renderSystem.cullScene(commandBuffer, frameIndex);
                // Start Renderpass
                renderer.beginSwapChainRenderPass(commandBuffer);
                // Draw Objects
                renderSystem.drawScene(commandBuffer, frameIndex); // DONT RUN THIS FUNCTION FOR NOW AS IT CURRENTLY CRASHES THE GPU DRIVERS
                // End Renderpass
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(device.getDevice());
    }

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
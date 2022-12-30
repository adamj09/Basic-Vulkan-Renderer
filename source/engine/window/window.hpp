#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <string>

namespace Renderer{
    class Window{
        public:
            Window(int width, int height, std::string name);
            ~Window();

            Window(const Window&) = delete;
		    Window& operator=(const Window&) = delete;

            void createWindowSurface(VkInstance instance, VkSurfaceKHR *pSurface);

            VkExtent2D getExtent(){ return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }

            bool shouldClose() { return glfwWindowShouldClose(window);}
            void resetWindowResizedFlag() { framebufferResized = false; }
            bool wasWindowResized() { return framebufferResized; }
            GLFWwindow* getGLFWwindow() { return window; }

        private:
            void createWindow();
            static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

            GLFWwindow* window;
            int width, height;
            std::string name;
            bool framebufferResized = false;
    };
}
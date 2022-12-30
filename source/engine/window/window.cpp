#include "window.hpp"

#include <stdexcept>

namespace Renderer{
    Window::Window(int width, int height, std::string name) 
    : width{width}, height{height}, name{name}{
        createWindow();
    }
    
    Window::~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Window::createWindow(){
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *pSurface){
        if(glfwCreateWindowSurface(instance, window, nullptr, pSurface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create surface.");
    }

    void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto newWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        newWindow->framebufferResized = true;
        newWindow->width = width;
        newWindow->height = height;
    }
}
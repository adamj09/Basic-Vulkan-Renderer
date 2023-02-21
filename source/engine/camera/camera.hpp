#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "engine/window/window.hpp"

namespace Renderer{
    struct Plane {
        glm::vec3 point {0.f, 0.f, 0.f};
        glm::vec3 normal {0.f, 1.f, 0.f};
    };

    struct BoundingBox {
        Plane nearPlane;
        Plane farPlane;

        Plane leftPlane;
        Plane rightPlane;

        Plane topPlane;
        Plane bottomPlane;
        void reset();
    };

    class Camera{
        public:
            struct KeyMappings {
                int moveLeft = GLFW_KEY_A;
                int moveRight = GLFW_KEY_D;
                int moveForward = GLFW_KEY_W;
                int moveBackward = GLFW_KEY_S;
                int moveUp = GLFW_KEY_SPACE;
                int moveDown = GLFW_KEY_LEFT_CONTROL;
                int lookLeft = GLFW_KEY_LEFT;
                int lookRight = GLFW_KEY_RIGHT;
                int lookUp = GLFW_KEY_UP;
                int lookDown = GLFW_KEY_DOWN;
            };

            //void setOrthographicProjection(float newLeft, float newRight, float newTop, float newBottom, float newNear, float newFar);
            void setPerspectiveProjection(float newFovy, float newAspect, float newNear, float newFar);
            void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
            void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
            void setViewYXZ(glm::vec3 position, glm::vec3 rotation);
            void moveInPlaneXZ(GLFWwindow* window, float dt);
            BoundingBox createFrustumViewBounds();

            const glm::mat4& getProjection() const { return projectionMatrix; }
            const glm::mat4& getView() const { return viewMatrix; }
            const glm::mat4& getInverseView() const { return inverseViewMatrix; }
            const glm::vec3 getPosition() const { return glm::vec3(inverseViewMatrix[3]); }

            bool enableFrustumCulling = true;

            float moveSpeed{0};
            float lookSpeed{0};

        private:
            glm::mat4 projectionMatrix{1.f};
            glm::mat4 viewMatrix{1.f};
            glm::mat4 inverseViewMatrix{1.f};

            glm::vec3 translation{0.f, 0.f, -2.5f};
            glm::vec3 rotation{0.f, 0.f, 0.f};  

            float fovy, aspect, near, far;

            glm::vec3 forwardDir;
            glm::vec3 rightDir;
            glm::vec3 upDir;

            KeyMappings keys{};
    };
}
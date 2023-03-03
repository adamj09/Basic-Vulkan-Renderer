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
            //void setOrthographicProjection(float newLeft, float newRight, float newTop, float newBottom, float newNear, float newFar);
            void setPerspectiveProjection(float newFovy, float newAspect, float newNear, float newFar);
            void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
            void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
            void setViewYXZ(glm::vec3 position, glm::vec3 rotation);
            BoundingBox createFrustumViewBounds();

            const glm::mat4& getProjection() const { return projectionMatrix; }
            const glm::mat4& getView() const { return viewMatrix; }
            const glm::mat4& getInverseView() const { return inverseViewMatrix; }
            const glm::vec3 getPosition() const { return glm::vec3(inverseViewMatrix[3]); }

            bool enableFrustumCulling = true;
        private:
            glm::mat4 projectionMatrix{1.f};
            glm::mat4 viewMatrix{1.f};
            glm::mat4 inverseViewMatrix{1.f};

            float fovy, aspect, near, far;

            glm::vec3 forwardDir;
            glm::vec3 rightDir;
            glm::vec3 upDir;

    };
}
#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <limits>
#include <iostream>

namespace Renderer{

    void BoundingBox::reset(){
        nearPlane = {glm::vec3{0.f, 1.f, 0.f}, glm::vec3{0.f, 0.f, 0.f}};
        farPlane = {glm::vec3{0.f, 1.f, 0.f}, glm::vec3{0.f, 0.f, 0.f}};
        leftPlane = {glm::vec3{0.f, 1.f, 0.f}, glm::vec3{0.f, 0.f, 0.f}};
        rightPlane = {glm::vec3{0.f, 1.f, 0.f}, glm::vec3{0.f, 0.f, 0.f}};
        topPlane = {glm::vec3{0.f, 1.f, 0.f}, glm::vec3{0.f, 0.f, 0.f}};
        bottomPlane = {glm::vec3{0.f, 1.f, 0.f}, glm::vec3{0.f, 0.f, 0.f}};
    }
    
    /*void Camera::setOrthographicProjection(float newLeft, float newRight, float newTop, float newBottom, float newNear, float newFar) {
        projectionMatrix = glm::mat4{1.0f};
        projectionMatrix[0][0] = 2.f / (newRight - newLeft);
        projectionMatrix[1][1] = 2.f / (newBottom - newTop);
        projectionMatrix[2][2] = 1.f / (newFar - newNear);
        projectionMatrix[3][0] = -(newRight + newLeft) / (newRight - newLeft);
        projectionMatrix[3][1] = -(newBottom + newTop) / (newBottom - newTop);
        projectionMatrix[3][2] = -newNear / (newFar - newNear);
    }*/

    void Camera::setPerspectiveProjection(float newFovy, float newAspect, float newNear, float newFar) {
        assert(glm::abs(newAspect - std::numeric_limits<float>::epsilon()) > 0.0f);
        fovy = newFovy;
        aspect = newAspect;
        near = newNear;
        far = newFar;

        const float tanHalfFovy = tan(fovy / 2.f);
        projectionMatrix = glm::mat4{0.0f};
        projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
        projectionMatrix[1][1] = 1.f / (tanHalfFovy);
        projectionMatrix[2][2] = far / (far - near);
        projectionMatrix[2][3] = 1.f;
        projectionMatrix[3][2] = -(far * near) / (far - near);
    }

    BoundingBox Camera::createFrustumViewBounds(){
        assert(aspect != 0 && "Cannot create frustum view bounds with invalid aspect ratio.");
        BoundingBox viewBounds;
        viewBounds.reset();

        const float halfVSide = far * tanf(fovy / 2.f);
        const float halfHSide = aspect * halfVSide;
        glm::vec3 forwardMultFar = far * forwardDir;
        glm::vec3 position = glm::vec3(inverseViewMatrix[3]);

        viewBounds.nearPlane = {position + near * forwardDir, forwardDir};
        viewBounds.farPlane = {position + forwardMultFar, -forwardDir};
        viewBounds.topPlane = {position, glm::cross(rightDir, forwardDir - upDir * halfVSide)};
        viewBounds.bottomPlane = {position, glm::cross(forwardMultFar + upDir * halfVSide, rightDir)};
        viewBounds.rightPlane = {position, glm::cross(forwardMultFar - rightDir * halfHSide, upDir)};
        viewBounds.leftPlane = {position, glm::cross(upDir, forwardMultFar + rightDir * halfHSide)};

        return viewBounds;
    }

    void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
        const glm::vec3 w{glm::normalize(direction)};
        const glm::vec3 u{glm::normalize(glm::cross(w, up))};
        const glm::vec3 v{glm::cross(w, u)};

        viewMatrix = glm::mat4{1.f};
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -glm::dot(u, position);
        viewMatrix[3][1] = -glm::dot(v, position);
        viewMatrix[3][2] = -glm::dot(w, position);

        inverseViewMatrix = glm::mat4{1.f};
        inverseViewMatrix[0][0] = u.x;
        inverseViewMatrix[0][1] = u.y;
        inverseViewMatrix[0][2] = u.z;
        inverseViewMatrix[1][0] = v.x;
        inverseViewMatrix[1][1] = v.y;
        inverseViewMatrix[1][2] = v.z;
        inverseViewMatrix[2][0] = w.x;
        inverseViewMatrix[2][1] = w.y;
        inverseViewMatrix[2][2] = w.z;
        inverseViewMatrix[3][0] = position.x;
        inverseViewMatrix[3][1] = position.y;
        inverseViewMatrix[3][2] = position.z;
    }       

    void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
        setViewDirection(position, target - position, up);
    }

    void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
        const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
        const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
        viewMatrix = glm::mat4{1.f};
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -glm::dot(u, position);
        viewMatrix[3][1] = -glm::dot(v, position);
        viewMatrix[3][2] = -glm::dot(w, position);

        inverseViewMatrix = glm::mat4{1.f};
        inverseViewMatrix[0][0] = u.x;
        inverseViewMatrix[0][1] = u.y;
        inverseViewMatrix[0][2] = u.z;
        inverseViewMatrix[1][0] = v.x;
        inverseViewMatrix[1][1] = v.y;
        inverseViewMatrix[1][2] = v.z;
        inverseViewMatrix[2][0] = w.x;
        inverseViewMatrix[2][1] = w.y;
        inverseViewMatrix[2][2] = w.z;
        inverseViewMatrix[3][0] = position.x;
        inverseViewMatrix[3][1] = position.y;
        inverseViewMatrix[3][2] = position.z;
    }
}
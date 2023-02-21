#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <limits>

namespace Renderer{

    void BoundingBox::reset(){
        nearPlane = {glm::vec3{0.f, 1.f, 0.f}, 0.f};
        farPlane = {glm::vec3{0.f, 1.f, 0.f}, 0.f};
        leftPlane = {glm::vec3{0.f, 1.f, 0.f}, 0.f};
        rightPlane = {glm::vec3{0.f, 1.f, 0.f}, 0.f};
        topPlane = {glm::vec3{0.f, 1.f, 0.f}, 0.f};
        bottomPlane = {glm::vec3{0.f, 1.f, 0.f}, 0.f};
    }
    
    void Camera::setOrthographicProjection(float newLeft, float newRight, float newTop, float newBottom, float newNear, float newFar) {
        projectionMatrix = glm::mat4{1.0f};
        projectionMatrix[0][0] = 2.f / (newRight - newLeft);
        projectionMatrix[1][1] = 2.f / (newBottom - newTop);
        projectionMatrix[2][2] = 1.f / (newFar - newNear);
        projectionMatrix[3][0] = -(newRight + newLeft) / (newRight - newLeft);
        projectionMatrix[3][1] = -(newBottom + newTop) / (newBottom - newTop);
        projectionMatrix[3][2] = -newNear / (newFar - newNear);
        setBoxViewBounds(newLeft, newRight, newTop, newBottom, newNear, newFar);
    }

    void Camera::setPerspectiveProjection(float newFovy, float newAspect, float newNear, float newFar) {
        assert(glm::abs(newAspect - std::numeric_limits<float>::epsilon()) > 0.0f);
        const float tanHalfFovy = tan(newFovy / 2.f);
        projectionMatrix = glm::mat4{0.0f};
        projectionMatrix[0][0] = 1.f / (newAspect * tanHalfFovy);
        projectionMatrix[1][1] = 1.f / (tanHalfFovy);
        projectionMatrix[2][2] = newFar / (newFar - newNear);
        projectionMatrix[2][3] = 1.f;
        projectionMatrix[3][2] = -(newFar * newNear) / (newFar - newNear);
        setFrustumViewBounds(newAspect, newNear, newFar);
    }

    void Camera::setBoxViewBounds(float left, float right, float top, float bottom, float near, float far){
        viewBounds.reset();

        glm::vec4 corners[8]{
            glm::vec4{-1.f, -1.f, -1.f, 1.f},  // A 0  Top left front corner
            glm::vec4{-1.f, -1.f, 1.f, 1.f},   // B 1  Top left back corner
            glm::vec4{1.f, -1.f, 1.f, 1.f},    // C 2  Top right back corner
            glm::vec4{1.f, -1.f, -1.f, 1.f},   // D 3  Top right front corner
            glm::vec4{-1.f, 1.f, -1.f, 1.f},   // E 4  Bottom left front corner
            glm::vec4{-1.f, 1.f, 1.f, 1.f},    // F 5  Bottom left back corner
            glm::vec4{1.f, 1.f, 1.f, 1.f},     // G 6  Bottom right back corner
            glm::vec4{1.f, 1.f, -1.f, 1.f}     // H 7  Bottom right front corner
        };

        // Transform the box corners
        (corners[0] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[1] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[2] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[3] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[4] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[5] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[6] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[7] * inverseViewMatrix) / inverseViewMatrix[3].w;
    }

    void Camera::setFrustumViewBounds(float aspect, float near, float far){
        viewBounds.reset();

        glm::vec4 corners[8]{
            glm::vec4{-1.f, -1.f, -1.f, 1.f},  // A 0  Top left front corner
            glm::vec4{-1.f, -1.f, 1.f, 1.f},   // B 1  Top left back corner
            glm::vec4{1.f, -1.f, 1.f, 1.f},    // C 2  Top right back corner
            glm::vec4{1.f, -1.f, -1.f, 1.f},   // D 3  Top right front corner
            glm::vec4{-1.f, 1.f, -1.f, 1.f},   // E 4  Bottom left front corner
            glm::vec4{-1.f, 1.f, 1.f, 1.f},    // F 5  Bottom left back corner
            glm::vec4{1.f, 1.f, 1.f, 1.f},     // G 6  Bottom right back corner
            glm::vec4{1.f, 1.f, -1.f, 1.f}     // H 7  Bottom right front corner
        };

        // Transform the box corners
        (corners[0] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[1] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[2] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[3] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[4] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[5] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[6] * inverseViewMatrix) / inverseViewMatrix[3].w;  
        (corners[7] * inverseViewMatrix) / inverseViewMatrix[3].w;

        glm::vec3 nearNormal = glm::cross(
            glm::vec3{corners[0].x - corners[4].x, corners[0].y - corners[4].y, corners[0].z - corners[4].z},   
            glm::vec3{corners[0].x - corners[3].x, corners[0].y - corners[3].y, corners[0].z - corners[3].z});
        glm::vec3 farNormal = glm::cross(
            glm::vec3{corners[2].x - corners[6].x, corners[2].y - corners[6].y, corners[2].z - corners[6].z}, 
            glm::vec3{corners[2].x - corners[1].x, corners[2].y - corners[1].y, corners[2].z - corners[1].z});
        glm::vec3 topNormal = glm::cross(
            glm::vec3{corners[3].x - corners[0].x, corners[3].y - corners[0].y, corners[3].z - corners[0].z}, 
            glm::vec3{corners[3].x - corners[2].x, corners[3].y - corners[2].y, corners[3].z - corners[2].z});
        glm::vec3 bottomNormal = glm::cross(
            glm::vec3{corners[7].x - corners[4].x, corners[7].y - corners[4].y, corners[7].z - corners[4].z}, 
            glm::vec3{corners[7].x - corners[6].x, corners[7].y - corners[6].y, corners[7].z - corners[6].z});
        glm::vec3 rightNormal = glm::cross(
            glm::vec3{corners[7].x - corners[3].x, corners[7].y - corners[3].y, corners[7].z - corners[3].z}, 
            glm::vec3{corners[7].x - corners[6].x, corners[7].y - corners[6].y, corners[7].z - corners[6].z});
        glm::vec3 leftNormal = glm::cross(
            glm::vec3{corners[0].x - corners[1].x, corners[0].y - corners[1].y, corners[0].z - corners[1].z}, 
            glm::vec3{corners[0].x - corners[4].x, corners[0].y - corners[4].y, corners[0].z - corners[4].z});

        glm::vec3 position = glm::vec3(inverseViewMatrix[3]);

        viewBounds.nearPlane = {nearNormal, near};
        viewBounds.farPlane = {farNormal, far};
        viewBounds.topPlane = {topNormal, 0.f};
        viewBounds.bottomPlane = {bottomNormal, 0.f};
        viewBounds.rightPlane = {rightNormal, 0.f};
        viewBounds.leftPlane = {leftNormal, 0.f};
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

    void Camera::moveInPlaneXZ(GLFWwindow* window, float dt) {
        glm::vec3 rotate{0};
        if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
        if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
        if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
        if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        // limit pitch values between +/- ~85 degrees
        rotation.x = glm::clamp(rotation.x, -1.5f, 1.5f);
        rotation.y = glm::mod(rotation.y, glm::two_pi<float>());

        float yaw = rotation.y;
        forwardDir = {sin(yaw), 0.f, cos(yaw)};
        rightDir = {forwardDir.z, 0.f, -forwardDir.x};
        upDir = {0.f, -1.f, 0.f};

        glm::vec3 moveDir{0.f};
        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
            translation += moveSpeed * dt * glm::normalize(moveDir);
    }
}